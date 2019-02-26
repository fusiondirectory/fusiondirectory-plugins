<?php
/*
 * This is a script to test the sinaps plugin by sending it an arbitrary XML file
 * Usage: php testEndPoint.php "http://url/fusiondirectory/sinaps.php?token=validtoken" /path/to/file.xml
 * */

sendPost(
  $argv[1],
  file_get_contents($argv[2])
);

function sendPost ($url, $xml)
{
  // performs the HTTP(S) POST
  $opts = [
    'http' => [
      'method'  => 'POST',
      'header'  => 'Content-type: application/xml',
      'content' => $xml,
    ],
    'ssl' => []
  ];

  $context  = stream_context_create($opts);
  $fp = fopenWithErrorHandling($url, 'r', FALSE, $context);
  if (!is_array($fp)) {
    $response = '';
    while ($row = fgets($fp)) {
      $response .= $row;
    }
    echo '***** Server response *****'."\n".$response.'***** End of server response *****'."\n";
  } else {
    if (!empty($fp)) {
      $errormsg = implode("\n", $fp);
    } else {
      $errormsg = 'Unable to connect to '.$url;
    }
    die($errormsg);
  }
  exit();
}

/* Calls fopen, gives errors as an array if any, file handle if successful */
function fopenWithErrorHandling (...$args)
{
  $errors = [];
  set_error_handler(
    function ($errno, $errstr, $errfile, $errline, $errcontext) use (&$errors)
    {
      $errors[] = $errstr;
    }
  );
  $fh = @fopen(...$args);
  restore_error_handler();
  if ($fh !== FALSE) {
    return $fh;
  }
  return $errors;
}
