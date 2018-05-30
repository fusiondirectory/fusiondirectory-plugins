<?php
require_once('../include/php_setup.inc');
require_once('functions.inc');
require_once('variables.inc');

$http_raw_post_data = file_get_contents('php://input');

if (!$http_raw_post_data) {
  return;
}

/* Parse configuration file */
$config = new config(CONFIG_DIR.'/'.CONFIG_FILE, $BASE_DIR);
/* Set config server to default one */
$directory = $config->data['MAIN']['DEFAULT'];
if (!isset($config->data['LOCATIONS'][$directory])) {
  $directory = key($config->data['LOCATIONS']);
}
$config->set_current($directory);
session::global_set('config', $config);

if ($config->get_cfg_value('fdSinapsEnabled') != 'TRUE') {
  returnError('SINAPS integration is disabled'."\n");
}

$request = new sinapsRequest($http_raw_post_data;);

if (($request->codeOperation() == 'DIFFUSION') && ($request->codeDomaine() == 'STRUCTURE')) {
  echo $request->acquittementTechnique(200, 'Diffusion de structure reçue')."\n";
  handleStructureDiffusion($request->getSupannEntiteValues());
} elseif (($request->codeOperation() == 'DIFFUSION') && ($request->codeDomaine() == 'PERSONNE')) {
  echo $request->acquittementTechnique(200, 'Diffusion de personne reçue')."\n";
  handlePersonneDiffusion($request->getUserValues());
} else {
  returnError('Cannot handle '.$request->codeDomaine().' '.$request->codeOperation().' '.$request->operationVersion()."\n");
}

exit();

/*TODO
 * Ignorer si date validité pas atteinte
 */

function returnError($request, $errorText)
{
  echo $request->acquittementTechnique(500, $errorText)."\n";
  error_log('Error: '.$errorText);
  exit();
}

function sendAcquittement($xml)
{
  global $config;

  $url  = $config->get_cfg_value('SinapsAcknowledgmentURL');
  $user = $config->get_cfg_value('SinapsLogin');
  $pwd  = $config->get_cfg_value('SinapsPassword');

  // performs the HTTP(S) POST
  $opts = array (
    'http' => array (
      'method'  => 'POST',
      'header'  => 'Content-type: application/xml'."\r\n".'Authorization: Basic '.base64_encode($user.':'.$pwd),
      'content' => $xml,
    ),
    'ssl' => array()
  );

  $context  = stream_context_create($opts);
  $fp = fopenWithErrorHandling($url, 'r', FALSE, $context);
  if (!is_array($fp)) {
    $response = '';
    while ($row = fgets($fp)) {
      $response .= trim($row)."\n";
    }
    //~ echo '***** Server response *****'."\n".$response.'***** End of server response *****'."\n";
  } else {
    if (!empty($fp)) {
      $errormsg = implode("\n", $fp);
    } else {
      $errormsg = 'Unable to connect to '.$url;
    }
    returnError($errormsg);
  }
  exit();
}

function handleStructureDiffusion($values)
{
  $uuid = $values['entite']['supannRefId'][0];
  $entites = objects::ls('entite', 'ou', NULL, '(supannRefId='.$uuid.')');
  $message = 'Entite created';
  if (!empty($entites)) {
    if (count($entites) > 1) {
      $error = 'Multiple entite matches id '.$uuid;
      sendAcquittement($request->acquittementFonctionnel(500, 10, $error, $dn));
      exit();
    } else {
      $dn = key($entites);
      $message = 'Entite updated';
    }
  } else {
    $dn = '';
  }
  $error = fillObject('entite', $values, $dn);
  if ($error !== TRUE) {
    sendAcquittement($request->acquittementFonctionnel(500, 10, $error, $uuid));
  } else {
    sendAcquittement($request->acquittementFonctionnel(200, 0, $message, $uuid));
  }
}

function handlePersonneDiffusion($values)
{
  $uuid = $values['supannAccount']['supannRefId'][0];
  $entites = objects::ls('user', 'ou', NULL, '(supannRefId='.$uuid.')');
  $message = 'User created';
  if (!empty($entites)) {
    if (count($entites) > 1) {
      $error = 'Multiple user matches id '.$uuid;
      sendAcquittement($request->acquittementFonctionnel(500, 10, $error, $dn));
      exit();
    } else {
      $dn = key($entites);
      $message = 'User updated';
    }
  } else {
    $dn = '';
  }
  $error = fillObject('user', $values, $dn);
  if ($error !== TRUE) {
    sendAcquittement($request->acquittementFonctionnel(500, 10, $error, $uuid));
  } else {
    sendAcquittement($request->acquittementFonctionnel(200, 0, $message, $uuid));
  }
}

function fillObject($type, $values, &$dn)
{
  if (empty($dn)) {
    $tabobject = objects::create($type);
  } else {
    $tabobject = objects::open($dn, $type);
  }
  foreach ($values as $tab => $tabvalues) {
    if (!isset($tabobject->by_object[$tab])) {
      return 'This tab does not exists: "'.$tab.'"';
    }
    if (is_subclass_of($tabobject->by_object[$tab], 'simplePlugin') &&
        $tabobject->by_object[$tab]->displayHeader &&
        !$tabobject->by_object[$tab]->is_account
      ) {
      list($disabled, $buttonText, $text) = $tabobject->by_object[$tab]->getDisplayHeaderInfos();
      if ($disabled) {
        return $text;
      }
      $tabobject->by_object[$tab]->is_account = TRUE;
    }
    $error = $tabobject->by_object[$tab]->deserializeValues($tabvalues);
    if ($error !== TRUE) {
      return $error;
    }
    $tabobject->current = $tab;
    $tabobject->save_object(); /* Should not do much as POST is empty, but in some cases may be needed */
  }
  $errors = $tabobject->save();
  if (empty($dn)) {
    $dn = $tabobject->dn;
  }
  if (!empty($errors)) {
    return $errors;
  }
  return TRUE;
}
