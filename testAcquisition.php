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

print_r($options);

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
  return $codeEntite;
}

try {
  /* We create the connection object */
  $client = new jsonRPCClient($options['url'], $http_options, $ssl_options, false);
  /* Then we need to login. Here we log in the default LDAP */
  $session_id = $client->login(NULL, $options['login'], $options['password']);

  $users = $client->ls($session_id, 'user', sinapsRequestAcquisiton::$attributes, $options['dn']);
  $user = reset($users);
  print_r($user);
  $request = new sinapsRequestAcquisiton();
  $request->fill($user, 'LDAPUUID', 'codeEntiteToldapUuidCallback');
  echo $request->getXml();

  $configuration = $client->ls(
    $session_id,
    'configuration',
    array(
      'fdSinapsAcquisitionURL'  => 1,
      'fdSinapsLogin'           => 1,
      'fdSinapsPassword'        => 1,
    )
  );
  print_r($configuration);
} catch (jsonRPCClientRequestErrorException $e) {
  die($e->getMessage());
} catch (jsonRPCClientNetworkErrorException $e) {
  die($e->getMessage());
}
