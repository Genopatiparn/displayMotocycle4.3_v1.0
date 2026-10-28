#pragma once

#include <Arduino.h>

const char wifimanager_html[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Wi-Fi setup</title>
  <style>
    body {
      box-sizing: border-box;
      max-width: 480px;
      margin: 40px auto;
      padding: 0 16px;
      font: 18px sans-serif;
    }

    input,
    button {
      box-sizing: border-box;
      width: 100%;
      margin: 8px 0;
      padding: 12px;
      font: inherit;
    }
  </style>
</head>
<body>
  <h1>Wi-Fi setup</h1>
  <form method="post" action="/ap-name">
    <label>
      Setup name 
      <input name="apSsid" maxlength="32">
    </label>
    <label>
      Setup password 
      <input name="apPassword" type="password" maxlength="63">
    </label>
    <button type="submit">Change name and password</button>
  </form>

  <hr>

<h2>DATA</h2>
<form method="post" action="/save">
  <label>Weight (kg)
    <input name="weight" type="number" step="0.1" min="0" required>
  </label>
  <label>Top Speed (km/h)
    <input name="topSpeed" type="number" min="0" required>
  </label>
  <label>Max RPM
    <input name="maxRpm" type="number" min="1000" max="30000" required>
  </label>
  <button type="submit">Save</button>
</form>

<h2>LED Style</h2>

<form method="post" action="/led-style">
  <button name="style" value="OFF">OFF</button>
  <button name="style" value="FLASH">FLASH</button>
  <button name="style" value="STROBE">STROBE</button>
  <button name="style" value="FADE">FADE</button>
  <button name="style" value="SMOOTH">SMOOTH</button>
  <button name="style" value="CUSTOM">CUSTOM</button>
  <button name="style" value="RPM">Shift RPM</button>
</form>

</body>
</html>
)rawliteral"; 