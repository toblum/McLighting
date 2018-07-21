#include <pgmspace.h>
char uploadspiffs_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang='en'>
<head>
  <title>McLighting SPIFFs Upload</title>
  <meta http-equiv='Content-Type' content='text/html; charset=utf-8' />
  <meta name='viewport' content='width=device-width' />
  <link rel='shortcut icon' type='image/x-icon' href='favicon.ico' />
</head>
<body>
<h3>Upload files to SPIFFs</h3><br>
<form method='POST' action='/upload' enctype='multipart/form-data'>
<input type='file' name='filename'>
<input type='submit' value='Upload'>
</form>
</body>
</html>
)=====";
