<?php
require_once('../include/php_setup.inc');
require_once('functions.inc');
require_once('variables.inc');

//~ $http_raw_post_data = file_get_contents('php://input');
$http_raw_post_data = file_get_contents($argv[1]);

function returnError($errorText)
{
  $protocol = (isset($_SERVER['SERVER_PROTOCOL']) ? $_SERVER['SERVER_PROTOCOL'] : 'HTTP/1.0');
  header($protocol.' 500 '.$errorText);
  error_log('Error: '.$errorText);
  exit();
}

if (!$http_raw_post_data) {
  return;
}

$compressmode = 'none';
if (strpos($http_raw_post_data, '<?xml') === 0) {
  $xml = $http_raw_post_data;
} elseif ($xml = @gzuncompress($http_raw_post_data)) {
  $compressmode = 'gzcompress';
}

if (strpos($xml, '<?xml') !== 0) {
  error_log('failed to extract XML content');
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
      <acq:Acquittement>
        <identifiantTransaction>c53c34b9-cfba-4ffc-9ce3-7e9142bb5334</identifiantTransaction>
        <ResponseCode>500</ResponseCode>
        <codeAcquittement>10</codeAcquittement>
        <messageAcquittement>Objet metier integre - Test U09</messageAcquittement>
        <identifiantObjApp>id02-objet-app</identifiantObjApp>
      </acq:Acquittement>
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
        'header'  => 'Content-type: application/xml'."\r\n".'Authorization: Basic '.base64_encode('admin:admin'),
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

$data = new SimpleXMLElement($xml);

$codeDomaine      = $data->domaine['code'];
$codeOperation    = $data->domaine->donnees->operation['codeOperation'];
$operationVersion = $data->domaine->donnees->operation['version'];
echo $codeDomaine.' '.$codeOperation.' '.$operationVersion."\n";
if (($codeOperation == 'DIFFUSION') && ($codeDomaine == 'STRUCTURE')) {
  handleStructureDiffusion($data->domaine->donnees->operation->structure);
} else {
  die('Cannot handle '.$codeDomaine.' '.$codeOperation.' '.$operationVersion."\n");
}

$reply = '';
switch ($compressmode) {
  case 'none':
    print $reply;
    break;

  case 'gzcompress':
    print gzcompress($reply);
    break;

  case 'gzencode':
    print gzencode($reply);
    break;

  case 'gzdeflate':
    print gzdeflate($reply);
    break;
}

function handleStructureDiffusion($structure)
{
  $mapping = array(
    'libelle20'       => 'ou',
    'descriptifLong'  => 'description',
    'codeStructure'   => 'supannCodeEntite',
    'codeNature'      => 'supannTypeEntite',
  );
  print_r($structure);

  //~ $tabObject  = objects::create('entite');
  //~ $baseObject = $tabObject->getBaseObject();
  foreach ($mapping as $sinapsAttr => $fdAttr) {
    if (isset($structure->$sinapsAttr)) {
      //~ $baseObject->$fdAttr = $structure->$sinapsAttr;
      echo "$fdAttr: ".$structure->$sinapsAttr."\n";
    }
  }

  foreach ($structure->methodesDeContact->methodeDeContact as $method) {
    switch ((string)$method->codeTypeMethodeContact) {
      case 'TEL':
        //~ $baseObject->telephoneNumber = $method->valeur;
        echo "telephoneNumber: ".$method->valeur."\n";
        break;
      case 'FAX':
        //~ $baseObject->facsimileTelephoneNumber = $method->valeur;
        echo "facsimileTelephoneNumber: ".$method->valeur."\n";
        break;
      case 'ADR':
        if ((string)$method->adresse->temoinAdressePrincipale != 'true') {
          echo 'Ignore adresse non principale'."\n";
          continue;
        }
        if (isset($method->adresse->adresseFR)) {
          $adresse = '';
          if (isset($method->adresse->adresseFR->batiment)) {
            $adresse .= $method->adresse->adresseFR->batiment;
            if (isset($method->adresse->adresseFR->etage)) {
              $adresse .= ', '.$method->adresse->adresseFR->etage;
            }
            $adresse .= "\n";
          }
          $adresse .= $method->adresse->adresseFR->numeroVoie;
          if (isset($method->adresse->adresseFR->codeBTQC)) {
            $adresse .= ' '.$method->adresse->adresseFR->codeBTQC;
          }
          $adresse .= ' '.$method->adresse->adresseFR->typeVoie;
          $adresse .= ' '.$method->adresse->adresseFR->nomVoie;
          $adresse .= "\n";
          if (isset($method->adresse->adresseFR->complement)) {
            $adresse .= $method->adresse->adresseFR->complement."\n";
          }
          if (isset($method->adresse->adresseFR->lieuDit)) {
            $adresse .= $method->adresse->adresseFR->lieuDit."\n";
          }
          $adresse .= $method->adresse->adresseFR->codePostal;
          $adresse .= ' '.$method->adresse->adresseFR->BD;
          $adresse .= "\n";
          $adresse .= "FRANCE\n";
        } elseif (isset($method->adresse->adresseETR)) {
          $adresse = '';
          if (isset($method->adresse->adresseETR->batiment)) {
            $adresse .= $method->adresse->adresseETR->batiment;
            if (isset($method->adresse->adresseETR->etage)) {
              $adresse .= ', '.$method->adresse->adresseETR->etage;
            }
            $adresse .= "\n";
          }
          $adresse .= $method->adresse->adresseETR->numeroVoie;
          $adresse .= ' '.$method->adresse->adresseETR->nomVoie;
          $adresse .= "\n";
          if (isset($method->adresse->adresseETR->complement)) {
            $adresse .= $method->adresse->adresseETR->complement."\n";
          }
          $adresse .= $method->adresse->adresseETR->codePostal;
          $adresse .= ' '.$method->adresse->adresseETR->ville;
          $adresse .= "\n";
          $adresse .= $method->adresse->adresseETR->etat."\n";
          $adresse .= $method->adresse->adresseETR->pays."\n";
        } else {
          echo 'Ignore adresse non FR non ETR'."\n";
          continue;
        }
        //~ $baseObject->postalAddress = $adresse;
        echo "postalAddress: ".$adresse."\n";
        break;
      default:
        echo 'ignoring codeTypeMethodeContact '.$method->codeTypeMethodeContact."\n";
    }
  }

  foreach ($structure->liensStructure->lienStructure as $lienStructure) {
    if (((string)$lienStructure->codeTypeLien == 'HIE') && isset($lienStructure->codeStructureMere)) {
      //TODO: vérifier date de début, peut-être prendre identifiantExterne
      //~ $baseObject->supannCodeEntiteParent = $lienStructure->codeStructureMere;
      echo "supannCodeEntiteParent: ".$lienStructure->codeStructureMere."\n";
    }
  }
  //~ $baseObject->base = $config->current['BASE'];
  //~ $message = $tabObject->save();
  //~ if (!empty($message)) {
    //~ print_r($message);
    //~ die();
  //~ }
  // lienStructure -> supannRefId
}
