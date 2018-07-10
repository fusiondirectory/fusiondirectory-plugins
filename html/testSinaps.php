<?php
require_once('../include/php_setup.inc');
require_once('functions.inc');
require_once('variables.inc');

//~ $http_raw_post_data = file_get_contents('php://input');
$http_raw_post_data = file_get_contents($argv[1]);

function returnError($request, $errorText)
{
  echo $request->acquittementTechnique(500, $errorText)."\n";
  error_log('Error: '.$errorText);
  exit();
}

if (!$http_raw_post_data) {
  return;
}

function testAcquittement()
{
  $ip   = '10.92.192.170';
  $port = 9058;
  $url  = 'http://'.$ip.':'.$port;
  $request = '<?xml version="1.0" encoding="UTF-8"?>
<socle:root xmlns:socle="http://socle.sinaps.amue.fr/V1_1" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:acq="http://referentiels.SINAPS.amue.fr/acquittementFonctionnel">
<domaine code="ACQUITTEMENT">
  <donnees>
    <operation codeOperation="CREATION" version="1.0">
      <Acquittement>
        <identifiantTransaction>c53c34b9-cfba-4ffc-9ce3-7e9142bb5334</identifiantTransaction>
        <ResponseCode>500</ResponseCode>
        <codeAcquittement>10</codeAcquittement>
        <messageAcquittement>Objet metier integre - Test U09</messageAcquittement>
        <identifiantObjApp>id02-objet-app</identifiantObjApp>
      </Acquittement>
    </operation>
  </donnees>
</domaine>
</socle:root>';
  $request = 'coucou';
  // performs the HTTP(S) POST
  $opts = array (
    'http' => array_merge(
      array (
        'method'  => 'POST',
        'header'  => 'Content-type: application/xml'."\r\n".'Authorization: Basic '.base64_encode('PASSPORT:Irste@92'),
        'content' => $request,
      ),
      []
    ),
    'ssl' => []
  );

  $context  = stream_context_create($opts);
  $fp = fopenWithErrorHandling($url, 'r', FALSE, $context);
  if (!is_array($fp)) {
    $response = '';
    while ($row = fgets($fp)) {
      $response .= trim($row)."\n";
    }
    echo '***** Server response *****'."\n".$response.'***** End of server response *****'."\n";
  } else {
    if (!empty($fp)) {
      $errormsg = implode("\n", $fp);
    } else {
      $errormsg = 'Unable to connect to '.$url;
    }
    throw new Exception($errormsg);
  }
  exit();
}

$request = new sinapsRequest($http_raw_post_data);

echo $request->codeDomaine().' '.$request->codeOperation().' '.$request->operationVersion()."\n";
if (($request->codeOperation() == 'DIFFUSION') && ($request->codeDomaine() == 'STRUCTURE')) {
  echo $request->acquittementTechnique(200, 'Diffusion de structure reçue')."\n";
  handleStructureDiffusion($request->getSupannEntiteValues());
} elseif (($request->codeOperation() == 'DIFFUSION') && ($request->codeDomaine() == 'PERSONNE')) {
  echo $request->acquittementTechnique(200, 'Diffusion de personne reçue')."\n";
  print_r($request->getUserValues());
} else {
  returnError('Cannot handle '.$request->codeDomaine().' '.$request->codeOperation().' '.$request->operationVersion()."\n");
}

/*TODO
 * Ignorer si date validité pas atteinte
 * Acquitter
 * Pousser dans FD
 * Détecter si création
 * Gérer conflit
 * Trancoder
 */

function handleStructureDiffusion($values)
{
  print_r($values);
  //~ $dn     = '';
  //~ $error  = fillObject('entite', $values, $dn);
  //~ if ($error !== TRUE) {
    //~ sendAcquittement($request->acquittementFonctionnel(500, 10, $error));
  //~ } else {
    //~ sendAcquittement($request->acquittementFonctionnel(200, 0, 'Entite created', $dn));
  //~ }
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
