<?php
/*
 * This is a script to test the sinaps acquisition by syncing a user
 */

/* You can find these files in FusionDirectory include directory */
require_once('/usr/share/fusiondirectory/include/jsonRPCClient.php');
require_once('/usr/share/fusiondirectory/include/functions.inc');
require_once('/usr/share/fusiondirectory/include/class_exceptions.inc');
require_once('include/class_sinapsRequest.inc');
require_once('include/class_sinapsRequestAcquisition.inc');
require_once('include/class_sinapsDiffusionHandlerJob.inc');


function usage($error = '')
{
  global $argv;
  if ($error) {
    echo "Error: $error\n";
  }
  echo "Usage: $argv[0] [-h | --help] --url=<url> --login=<login> [-P | --password=<password>] --dn=<dn> [--ca=<ca_file>] [--server=<ldap_server>]\n";
  exit();
}

$options = getopt(
  'Ph',
  array(
    'help',
    'url:',
    'ca:',
    'login:',
    'password:',
    'server:',
    'dn:',
  )
);

if (isset($options['h']) || isset($options['help'])) {
  usage();
}

if (!isset($options['login'])) {
  usage('Missing login information');
}

if (!isset($options['dn'])) {
  usage('Missing target dn');
}

if (isset($options['P'])) {
  $options['password'] = readline('password: ');
} elseif (!isset($options['password'])) {
  usage('You need to specify option P or password');
}

if (isset($options['url'])) {
  if (!preg_match('/jsonrpc\.php$/', $options['url'])) {
    if (!preg_match('|/$|', $options['url'])) {
      $options['url'] .= '/';
    }
    $options['url'] .= 'jsonrpc.php';
  }
} else {
  $options['url'] = 'https://localhost/fusiondirectory/jsonrpc.php';
}

$ssl_options = array(
);
$http_options = array(
  'timeout' => 10
);

if (isset($options['ca'])) {
  $ssl_options['cafile'] = $options['ca'];
}

function codeEntiteToldapUuidCallback($codeEntite)
{
  global $client, $session_id, $configuration;

  /* Translate code entite to UUID refs
   * This is done here to make sure sinapsRequestAcquisition does not perform LDAP searches by itself
   * TODO add a cache
   * */
  $entites = $client->ls(
    $session_id,
    'entite',
    array('supannRefId' => '*', 'supannTypeEntite' => 1, 'dn' => 'raw'),
    NULL,
    '(supannCodeEntite='.$codeEntite.')'
  );
  if (empty($entites)) {
    $error = 'Could not find entity '.$codeEntite;
    die ($error);
  } elseif (count($entites) > 1) {
    $error = 'Multiple entite matches codeEntite '.$codeEntite;
    die ($error);
  } else {
    $entite = reset($entites);
    foreach ($entite['supannRefId'] as $supannRefId) {
      if (preg_match('/^{'.preg_quote($configuration['fdSinapsUuidPrefix'], '/').'}(.+)$/', $supannRefId, $m)) {
        return $m[1];
      }
    }
    $error = 'Could not find any UUID for '.$entite['dn'];
    die ($error);
  }
}

try {
  /* We create the connection object */
  $client = new jsonRPCClient($options['url'], $http_options, $ssl_options, FALSE);
  /* Then we need to login. Here we log in the default LDAP */
  $session_id = $client->login(NULL, $options['login'], $options['password']);

  $configurations = $client->ls(
    $session_id,
    'configuration',
    array(
      'fdSinapsEnabled'                     => 1,
      'fdSinapsAcquisitionURL'              => 1,
      'fdSinapsLogin'                       => 1,
      'fdSinapsPassword'                    => 1,
      'fdSinapsUuidPrefix'                  => 1,
      'fdSinapsAcquisitonTypeExterne'       => 1,
      'fdSinapsAcquisitionContactMethodMap' => '*',
    )
  );
  $configuration = reset($configurations);
  if (isset($configuration['fdSinapsEnabled']) && ($configuration['fdSinapsEnabled'] == 'FALSE')) {
    usage('Sinaps integration is disabled in FusionDirectory configuration');
  }
  if (!isset($configuration['fdSinapsPassword'])) {
    usage('Sinaps password is not filled in FusionDirectory configuration');
  }
  if (!isset($configuration['fdSinapsAcquisitionURL'])) {
    usage('Sinaps acquisition URL is not filled in FusionDirectory configuration');
  }
  if (!isset($configuration['fdSinapsUuidPrefix'])) {
    $configuration['fdSinapsUuidPrefix'] = 'LDAPUUID';
  }
  if (!isset($configuration['fdSinapsLogin'])) {
    $configuration['fdSinapsLogin'] = 'fusiondirectory';
  }
  if (!isset($configuration['fdSinapsAcquisitonTypeExterne'])) {
    $configuration['fdSinapsAcquisitonTypeExterne'] = 'FD';
  }

  $attributes = sinapsRequestAcquisiton::$attributes;

  $mapping = array();
  foreach ($configuration['fdSinapsAcquisitionContactMethodMap'] as $field) {
    list($ldapField, $sinapsType) = explode('|', $field, 2);

    $mapping[$ldapField]    = $sinapsType;
    $attributes[$ldapField] = 1;
  }

  $users = $client->ls($session_id, 'user', $attributes, $options['dn']);
  $user = reset($users);

  $request = new sinapsRequestAcquisiton();
  $request->fill($user, $configuration['fdSinapsUuidPrefix'], $configuration['fdSinapsAcquisitonTypeExterne'], $mapping, 'codeEntiteToldapUuidCallback');
  $xml = $request->getXml();
  echo "Request:\n$xml\n";

  $answer = sinapsDiffusionHandlerJob::sendPostRequest($configuration['fdSinapsAcquisitionURL'], $configuration['fdSinapsLogin'], $configuration['fdSinapsPassword'], $xml);

  echo "Answer:\n$answer\n";

} catch (jsonRPCClientRequestErrorException $e) {
  die($e->getMessage());
} catch (jsonRPCClientNetworkErrorException $e) {
  die($e->getMessage());
}
