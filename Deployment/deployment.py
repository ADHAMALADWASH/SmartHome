import pandas as pd
from joblib import load
from sklearn.base import BaseEstimator, TransformerMixin
import serial
import time

# ---------- تحميل الموديلات ----------


# ---------- دالة تحويل الثواني لساعة:دقيقة:ثانية ----------
def sec_to_hms(seconds):
    hours = int(seconds // 3600)
    minutes = int((seconds % 3600) // 60)
    secs = int(seconds % 60)
    return f"{hours:02d}:{minutes:02d}:{secs:02d}"

# ---------- Feature Engineering ----------
class FeatureEngineering:
    def __init__(self, df, wet_threshold=200):
        self.df = df.copy()
        self.wet_threshold = wet_threshold

    def preprocess(self):
        self.df['time'] = pd.to_datetime(self.df['time'], format='%H:%M:%S', errors='coerce')
        self.df['state'] = self.df['state'].map({'dry':0, 'wet':1})
        self.df['soil_type'] = self.df['soil_type'].astype('category').cat.codes
        self.df['moisture_percent'] = self.df['moisture_percent'].ffill().fillna(0)
        self.df['time'] = self.df['time'].ffill()
        self.df['pump_state'] = self.df['pump_state'].fillna(0)
        self.df['state'] = self.df['state'].fillna(0)
        self.df['soil_type'] = self.df['soil_type'].fillna(0)
        return self.df

    def compute_time_to_dry(self):
        self.df['time_to_dry_sec'] = 0.0
        last_slope = -0.5
        for i in range(len(self.df)):
            window = self.df.iloc[max(0, i-5):i+1]
            window = window[window['pump_state']==0]
            if len(window) >= 2:
                x = (window['time'] - window['time'].iloc[0]).dt.total_seconds()
                y = window['moisture_percent']
                slope = (y.iloc[-1] - y.iloc[0]) / (x.iloc[-1] - x.iloc[0])
                if abs(slope) < 0.01:
                    slope = last_slope
                else:
                    last_slope = slope
                seconds_to_dry = abs((1000 - y.iloc[-1]) / slope)
                self.df.at[i, 'time_to_dry_sec'] = seconds_to_dry
        self.df['time_to_dry_sec'] = self.df['time_to_dry_sec'].ffill().fillna(0)
        return self.df

    def get_features_and_target(self):
        self.preprocess()
        self.compute_time_to_dry()
        feature_cols = ['moisture_percent', 'pump_state', 'soil_type', 'state']
        X = self.df[feature_cols].astype(float)
        y = self.df['time_to_dry_sec'].astype(float)
        return X, y

class FeatureEngineeringTransformer(BaseEstimator, TransformerMixin):
    def fit(self, X, y=None):
        return self
    def transform(self, X):
        fe = FeatureEngineering(X)
        X_new, _ = fe.get_features_and_target()
        return X_new
anomaly_model = load(r"D:\Home System\Model\errormodel.pkl")
pipeline = load(r"D:\Home System\Model\pipeline.pkl")
# ---------- الاتصال بالـ Arduino ----------
ser = serial.Serial('COM4', 9600, timeout=1)
time.sleep(2)

data_buffer = []
last_predict_time = time.time()
PREDICT_INTERVAL = 3  # كل 3 ثواني

while True:
    line = ser.readline().decode('utf-8').strip()
    if line:
        parts = line.split('\t')
        if len(parts) == 6:
            soil_type, state, moisture_percent, t, pump_state, ldr_value = parts
            data_buffer.append({
                'soil_type': soil_type,
                'state': state,
                'moisture_percent': float(moisture_percent),
                'time': t,
                'pump_state': int(pump_state),
                'ldr': int(ldr_value)
            })

    # ---------- التنبؤ DryTime و Status ----------
    if time.time() - last_predict_time >= PREDICT_INTERVAL and len(data_buffer) > 0:
        last_predict_time = time.time()
        df_new = pd.DataFrame([data_buffer[-1]])

        # تنبؤ الوقت للجفاف
        predictions = pipeline.predict(df_new)
        pred = predictions[0]
        hms = sec_to_hms(pred)

        # تنبؤ الخطأ / الحالة
        anomaly_features = df_new[['moisture_percent', 'pump_state', 'soil_type']]
        anomaly_pred = anomaly_model.predict(anomaly_features)[0]
        status_msg = "Error" if anomaly_pred == -1 else "No Error"

        # ---------- عرض النتائج ----------
        print("\n------ New Prediction ------")
        print(f"Predicted DryTime = {pred:.2f} sec ({hms})")
        print(f"Status = {status_msg}")
        print("----------------------------\n")

        # ---------- إرسال DryTime و Status للأردوينو ----------
        ser.write((str(int(pred)) + "\t" + status_msg + "\n").encode())
