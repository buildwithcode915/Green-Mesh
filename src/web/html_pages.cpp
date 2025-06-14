#include "html_pages.h"

String HTMLPages::getSetupPage() {
    return R"rawliteral(
<!DOCTYPE html>
<html><head>
    <title>Green Mesh Setup</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="UTF-8">
    <style>
        * { box-sizing: border-box; }
        body { 
            font-family: Arial, sans-serif; 
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            display: flex; 
            justify-content: center; 
            align-items: center; 
            min-height: 100vh; 
            margin: 0; 
            padding: 20px;
        }
        .form-container { 
            background: white; 
            padding: 30px; 
            border-radius: 15px; 
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.2); 
            width: 100%;
            max-width: 400px;
        }
        h2 { 
            text-align: center; 
            color: #333; 
            margin-bottom: 30px;
            font-size: 24px;
        }
        .form-group {
            margin-bottom: 20px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            color: #555;
            font-weight: bold;
        }
        input { 
            width: 100%; 
            padding: 12px; 
            border: 2px solid #ddd; 
            border-radius: 8px; 
            font-size: 16px;
            transition: border-color 0.3s;
        }
        input:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
        }
        button { 
            width: 100%; 
            padding: 15px; 
            background: #28a745; 
            color: white; 
            border: none; 
            border-radius: 8px;
            cursor: pointer; 
            font-weight: bold; 
            font-size: 16px;
            transition: background-color 0.3s;
        }
        button:hover { 
            background: #218838; 
        }
        button:disabled {
            background: #ccc;
            cursor: not-allowed;
        }
        .status { 
            text-align: center; 
            margin-top: 15px; 
            padding: 10px;
            border-radius: 5px;
            display: none;
        }
        .status.error {
            background: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
        .status.success {
            background: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        .spinner {
            display: none;
            width: 20px;
            height: 20px;
            border: 2px solid #ffffff;
            border-top: 2px solid transparent;
            border-radius: 50%;
            animation: spin 1s linear infinite;
            margin-right: 10px;
        }
        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
        .device-info {
            background: #e9ecef;
            padding: 15px;
            border-radius: 8px;
            margin-bottom: 20px;
            font-size: 14px;
            text-align: center;
        }
    </style>
</head>
<body>
    <div class='form-container'>
        <h2>🌱 Green Mesh Setup</h2>
        <div class="device-info">
            Enter your WiFi credentials and device information to get started.
        </div>
        <form id="setupForm" action="/save" method="POST">
            <div class="form-group">
                <label for="ssid">WiFi Network Name (SSID)</label>
                <input type="text" id="ssid" name="ssid" placeholder="Enter WiFi name" required maxlength="32">
            </div>
            <div class="form-group">
                <label for="password">WiFi Password</label>
                <input type="password" id="password" name="password" placeholder="Enter WiFi password" required minlength="8" maxlength="63">
            </div>
            <div class="form-group">
                <label for="customer_uid">User ID</label>
                <input type="text" id="customer_uid" name="customer_uid" placeholder="Enter your User ID" required>
            </div>
            <div class="form-group">
                <label for="device_number">Device ID</label>
                <input type="text" id="device_number" name="device_number" placeholder="Enter Device ID" required>
            </div>
            <button type="submit" id="submitBtn">
                <span class="spinner" id="spinner"></span>
                <span id="btnText">Save & Connect</span>
            </button>
        </form>
        <div class="status" id="status"></div>
    </div>

    <script>
        document.getElementById('setupForm').addEventListener('submit', function(e) {
            const submitBtn = document.getElementById('submitBtn');
            const spinner = document.getElementById('spinner');
            const btnText = document.getElementById('btnText');
            const status = document.getElementById('status');
            
            // Show loading state
            submitBtn.disabled = true;
            spinner.style.display = 'inline-block';
            btnText.textContent = 'Connecting...';
            status.style.display = 'none';
            
            // Basic validation
            const ssid = document.getElementById('ssid').value.trim();
            const password = document.getElementById('password').value;
            const customerUid = document.getElementById('customer_uid').value.trim();
            const deviceNumber = document.getElementById('device_number').value.trim();
            
            if (!ssid || !password || !customerUid || !deviceNumber) {
                e.preventDefault();
                showStatus('Please fill in all fields', 'error');
                resetButton();
                return;
            }
            
            if (password.length < 8) {
                e.preventDefault();
                showStatus('WiFi password must be at least 8 characters', 'error');
                resetButton();
                return;
            }
        });
        
        function showStatus(message, type) {
            const status = document.getElementById('status');
            status.textContent = message;
            status.className = 'status ' + type;
            status.style.display = 'block';
        }
        
        function resetButton() {
            const submitBtn = document.getElementById('submitBtn');
            const spinner = document.getElementById('spinner');
            const btnText = document.getElementById('btnText');
            
            submitBtn.disabled = false;
            spinner.style.display = 'none';
            btnText.textContent = 'Save & Connect';
        }
    </script>
</body></html>
    )rawliteral";
}

String HTMLPages::getConnectingPage() {
    return R"rawliteral(
<!DOCTYPE html>
<html><head>
    <title>Connecting...</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="UTF-8">
    <style>
        body { 
            font-family: Arial, sans-serif; 
            text-align: center; 
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            margin: 0;
            padding: 20px;
        }
        .container { 
            background: rgba(255, 255, 255, 0.95); 
            color: #333;
            max-width: 500px; 
            margin: 0 auto; 
            padding: 40px; 
            border-radius: 15px; 
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.2);
        }
        .loader { 
            border: 4px solid #f3f3f3; 
            border-top: 4px solid #28a745; 
            border-radius: 50%; 
            width: 60px; 
            height: 60px; 
            animation: spin 1s linear infinite; 
            margin: 30px auto; 
        }
        @keyframes spin { 
            0% { transform: rotate(0deg); } 
            100% { transform: rotate(360deg); } 
        }
        h2 {
            color: #28a745;
            margin-bottom: 20px;
        }
        .steps {
            text-align: left;
            margin: 30px 0;
            padding: 20px;
            background: #f8f9fa;
            border-radius: 8px;
        }
        .steps h3 {
            margin-top: 0;
            color: #495057;
        }
        .steps ol {
            margin: 0;
            padding-left: 20px;
        }
        .steps li {
            margin: 10px 0;
            color: #6c757d;
        }
        .warning {
            background: #fff3cd;
            color: #856404;
            padding: 15px;
            border-radius: 8px;
            margin: 20px 0;
            border-left: 4px solid #ffc107;
        }
    </style>
    <script>
        // Auto-close window after 30 seconds if possible
        setTimeout(function() {
            try {
                window.close();
            } catch(e) {
                console.log("Cannot auto-close window");
            }
        }, 30000);
    </script>
</head>
<body>
    <div class="container">
        <h2>✅ Settings Saved Successfully!</h2>
        <p>Your device is now restarting and will attempt to connect to your WiFi network.</p>
        <div class="loader"></div>
        
        <div class="steps">
            <h3>Next Steps:</h3>
            <ol>
                <li>The device will restart (takes about 10-15 seconds)</li>
                <li>It will connect to your home WiFi network</li>
                <li>The device will validate with our servers</li>
                <li>Once complete, you can access the device on your home network</li>
            </ol>
        </div>
        
        <div class="warning">
            <strong>Important:</strong> Please reconnect your phone/computer to your home WiFi network now.
        </div>
        
        <p><small>This window will automatically close in 30 seconds.</small></p>
    </div>
</body></html>
    )rawliteral";
}

String HTMLPages::getSuccessPage(const String& deviceNumber, const String& customerUid, 
                                const String& ssid, const String& ipAddress) {
    return R"rawliteral(
<!DOCTYPE html>
<html><head>
    <title>Green Mesh Setup Complete</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="UTF-8">
    <style>
        body { 
            font-family: Arial, sans-serif; 
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            display: flex; 
            justify-content: center; 
            align-items: center; 
            min-height: 100vh; 
            margin: 0; 
            padding: 20px;
        }
        .success-container { 
            background: white; 
            padding: 40px; 
            border-radius: 15px; 
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.2); 
            width: 100%;
            max-width: 500px; 
            text-align: center; 
        }
        h2 { 
            color: #28a745; 
            margin-bottom: 20px;
            font-size: 28px;
        }
        .checkmark { 
            color: #28a745; 
            font-size: 80px; 
            margin: 20px 0; 
            animation: pulse 2s infinite;
        }
        @keyframes pulse {
            0% { transform: scale(1); }
            50% { transform: scale(1.1); }
            100% { transform: scale(1); }
        }
        .details { 
            background: #f8f9fa; 
            padding: 20px; 
            border-radius: 10px; 
            margin: 30px 0; 
            text-align: left; 
        }
        .details h3 {
            margin-top: 0;
            color: #495057;
            text-align: center;
        }
        .details p { 
            margin: 10px 0; 
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .device-info { 
            font-weight: bold; 
            color: #495057;
            min-width: 100px;
        }
        .device-value {
            color: #28a745;
            font-family: monospace;
            background: #e9ecef;
            padding: 4px 8px;
            border-radius: 4px;
        }
        .actions {
            margin-top: 30px;
        }
        .btn {
            display: inline-block;
            padding: 12px 24px;
            margin: 5px;
            background: #28a745;
            color: white;
            text-decoration: none;
            border-radius: 8px;
            font-weight: bold;
            transition: background-color 0.3s;
        }
        .btn:hover {
            background: #218838;
        }
        .btn-secondary {
            background: #6c757d;
        }
        .btn-secondary:hover {
            background: #545b62;
        }
        .status-indicator {
            display: inline-block;
            width: 12px;
            height: 12px;
            background: #28a745;
            border-radius: 50%;
            margin-right: 8px;
            animation: blink 2s infinite;
        }
        @keyframes blink {
            0%, 50% { opacity: 1; }
            51%, 100% { opacity: 0.3; }
        }
    </style>
    <script>
        function refreshStatus() {
            fetch('/status')
            .then(response => response.json())
            .then(data => {
                console.log('Device status:', data);
                document.getElementById('heap').textContent = Math.round(data.heap_free / 1024) + ' KB';
                document.getElementById('rssi').textContent = data.wifi_rssi + ' dBm';
            })
            .catch(error => console.log('Status update failed:', error));
        }
        
        // Update status every 30 seconds
        setInterval(refreshStatus, 30000);
        
        // Initial status load
        window.onload = refreshStatus;
    </script>
</head>
<body>
    <div class='success-container'>
        <div class="checkmark">✓</div>
        <h2><span class="status-indicator"></span>Setup Complete!</h2>
        <p>Your Green Mesh device is successfully connected and activated.</p>
        
        <div class="details">
            <h3>Device Information</h3>
            <p><span class="device-info">Device ID:</span> <span class="device-value">)rawliteral" + deviceNumber + R"rawliteral(</span></p>
            <p><span class="device-info">User ID:</span> <span class="device-value">)rawliteral" + customerUid + R"rawliteral(</span></p>
            <p><span class="device-info">WiFi Network:</span> <span class="device-value">)rawliteral" + ssid + R"rawliteral(</span></p>
            <p><span class="device-info">IP Address:</span> <span class="device-value">)rawliteral" + ipAddress + R"rawliteral(</span></p>
            <p><span class="device-info">Free Memory:</span> <span class="device-value" id="heap">Loading...</span></p>
            <p><span class="device-info">WiFi Signal:</span> <span class="device-value" id="rssi">Loading...</span></p>
        </div>
        
        <div class="actions">
            <a href="/status" class="btn">View Status</a>
            <a href="#" class="btn btn-secondary" onclick="window.location.reload()">Refresh</a>
        </div>
        
        <p style="margin-top: 30px; color: #6c757d; font-size: 14px;">
            Your device is now operational. To reconfigure, press and hold the reset button on the device.
        </p>
    </div>
</body></html>
    )rawliteral";
}