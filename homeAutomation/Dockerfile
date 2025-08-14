FROM python:3.11-slim

WORKDIR /app

# Install dependencies
COPY requirements.txt .
RUN pip install -r requirements.txt

# Copy application
COPY mqtt_to_db.py .

CMD ["python", "mqtt_to_db.py"]
