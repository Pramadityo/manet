@echo off
title Smart Maggot Farming Launcher
echo ==========================================
echo Starting Smart Maggot Farming Application...
echo ==========================================

:: 1. Start FastAPI Backend Server in a new window
echo Launching Backend Server (FastAPI)...
start "Maggot Backend Server" cmd /k "python backend_server.py"

:: 2. Start MQTT Mock Broker in a new window (Optional, if you want local simulated data)
echo Launching MQTT Mock Broker...
start "MQTT Mock Broker" cmd /k "python mqtt_mock_broker.py"

:: 3. Start Frontend Dev Server (Vite) in the current window
echo Launching Frontend Server (Vite)...
cd frontend
npm run dev
