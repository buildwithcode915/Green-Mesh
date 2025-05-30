#include "html_pages.h"

String HTMLPages::getSetupPage() {
    return R"rawliteral(
      <html><head><title>Green Mesh Setup</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body { font-family: Arial; background: #f4f4f4; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }
        .form-container { background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1); width: 300px; }
        h2 { text-align: center; color: #333; }
        input, button { width: 100%; padding: 10px; margin: 8px 0; border-radius: 5px; box-sizing: border-box; }
        input { border: 1px solid #ddd; }
        button { background: #28a745; color: white; border: none; cursor: pointer; font-weight: bold; }
        button:hover { background: #218838; }
        .status { text-align: center; margin-top: 10px; }
      </style></head>
      <body><div class='form-container'>
      <h2>Green Mesh Setup</h2>
      <form action="/save" method="POST">
        <input type="text" name="ssid" placeholder="WiFi SSID" required><br>
        <input type="password" name="password" placeholder="WiFi Password" required><br>
        <input type="text" name="customer_uid" placeholder="User ID" required><br>
        <input type="text" name="device_number" placeholder="Device ID" required><br>
        <button type="submit">Save & Connect</button>
      </form>
      <div class="status"></div>
      </div></body></html>
    )rawliteral";
}

String HTMLPages::getConnectingPage() {
    return R"rawliteral(
      <html><head><title>Connecting...</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body { font-family: Arial; text-align: center; margin-top: 50px; background: #f4f4f4; }
        .container { background: white; max-width: 400px; margin: 0 auto; padding: 30px; border-radius: 10px; box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1); }
        .loader { border: 5px solid #f3f3f3; border-top: 5px solid #28a745; border-radius: 50%; width: 50px; height: 50px; animation: spin 2s linear infinite; margin: 20px auto; }
        @keyframes spin { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }
      </style>
      </head>
      <body>
        <div class="container">
          <h2>Settings Saved!</h2>
          <p>The device will now restart and attempt to connect to your WiFi.</p>
          <div class="loader"></div>
          <p>This page will automatically close. Please reconnect to your home WiFi.</p>
        </div>
      </body></html>
    )rawliteral";
}

String HTMLPages::getSuccessPage(const String& deviceNumber, const String& customerUid, 
                                const String& ssid, const String& ipAddress) {
    return R"rawliteral(
      <html><head><title>Green Mesh Setup Complete</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body { font-family: Arial; background: #f4f4f4; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }
        .success-container { background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1); width: 350px; text-align: center; }
        h2 { color: #28a745; }
        .checkmark { color: #28a745; font-size: 80px; margin: 10px 0; }
        .details { background: #f8f9fa; padding: 15px; border-radius: 5px; margin: 20px 0; text-align: left; }
        .details p { margin: 5px 0; }
        .device-info { font-weight: bold; }
      </style></head>
      <body><div class='success-container'>
      <div class="checkmark">✓</div>
      <h2>Setup Complete!</h2>
      <p>Your Green Mesh device is successfully connected and activated.</p>
      <div class="details">
        <p><span class="device-info">Device ID:</span> )rawliteral" + deviceNumber + R"rawliteral(</p>
        <p><span class="device-info">User ID:</span> )rawliteral" + customerUid + R"rawliteral(</p>
        <p><span class="device-info">WiFi Network:</span> )rawliteral" + ssid + R"rawliteral(</p>
        <p><span class="device-info">IP Address:</span> )rawliteral" + ipAddress + R"rawliteral(</p>
      </div>
      <p>You can now close this page and start using your Green Mesh device.</p>
      <p>If you need to reconfigure, press the reset button on the device.</p>
      </div></body></html>
    )rawliteral";
}