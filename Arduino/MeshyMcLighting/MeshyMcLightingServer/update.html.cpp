#include <pgmspace.h>
char update_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang='en'>
<head>
  <title>MeshyMcLighting Firware Update</title>
  <meta http-equiv='Content-Type' content='text/html; charset=utf-8' />
  <meta name='viewport' content='width=device-width' />
  <link rel='shortcut icon' type='image/x-icon' href='favicon.ico' />
</head>
<body>
<h3>Update MeshyMcLighting</h3><br>
<form method='POST' action='/update' enctype='multipart/form-data'>
<input type='file' name='update'>
<input type='submit' value='Update'>
</form>
</body>
</html>
)=====";
