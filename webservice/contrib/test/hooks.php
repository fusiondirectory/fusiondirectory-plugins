<?php

use Dredd\Hooks;
require_once('restclient.php');

$stash = [];

$successfullLogin = '/login > Login to the API > 200 > application/json';
$logout           = '/logout > End the currently opened session > 204';
$userCreate       = '/objects/{type} > Create an object of specific type > 201 > application/json';
$recoveryGet      = '/recovery > Generate recovery token for a user > 200 > application/json';
$recoveryUse      = '/recovery > Change a user password using a recovery token > 204';

$webserviceInfos  = [
  'ldapBase'  => 'dc=fusiondirectory,dc=org',
  'endPoint'  => 'http://localhost/fusiondirectory/rest.php/v1',
  'user'      => 'fd-admin',
  'password'  => 'adminpwd',
];

function connectRestClient (string $webserviceRestHost, string $user, string $pwd): RestClient
{
  $webserviceClient = new RestClient([
    'base_url'  => $webserviceRestHost,
  ]);
  /* Make sure json objects decode to associative arrays */
  $webserviceClient->register_decoder('json', function($data) {
    return json_decode($data, TRUE);
  });
  /* Then we need to login. Here we log in the default LDAP */
  $result = $webserviceClient->post('/login', json_encode(['user' => $user, 'password' => $pwd]));
  if ($result->info->http_code == 200) {
    $webserviceSessionId = $result->decode_response();
  } else {
    throw new Exception('Connection to REST API failed: '.$result->info->http_code.' '.$result->response);
  }

  $webserviceClient->set_option('headers', ['Session-Token' => $webserviceSessionId]);

  return $webserviceClient;
}

function callRestMethod (RestClient $webserviceClient, string $method, string $endpoint, $input = NULL)
{
  if (($method !== 'GET') && ($input !== NULL)) {
    $input = json_encode($input);
  }
  return $webserviceClient->execute($endpoint, $method, $input);
}

Hooks::before($successfullLogin, function(&$transaction) use ($webserviceInfos) {
  $requestBody = [];

  $requestBody['user']      = $webserviceInfos['user'];
  $requestBody['password']  = $webserviceInfos['password'];


  $transaction->request->body = json_encode($requestBody);
});

Hooks::after($successfullLogin, function(&$transaction) use (&$stash) {
  $parsedBody     = json_decode($transaction->real->body, TRUE);
  $stash['token'] = $parsedBody;
});

Hooks::after($recoveryGet, function(&$transaction) use (&$stash) {
  $parsedBody         = json_decode($transaction->real->body, TRUE);
  $stash['recovery']  = $parsedBody;
});

Hooks::before($recoveryUse, function(&$transaction) use (&$stash) {
  $requestBody = [
    'token'     => (string)($stash['recovery']['token']),
    'login'     => (string)($stash['recovery']['login']),
    'password1' => 'newpassword',
    'password2' => 'newpassword',
  ];

  $transaction->request->body = json_encode($requestBody);
});

Hooks::before($logout, function(&$transaction) {
  // Skip logout to avoid destroying the session
  $transaction->skip = TRUE;
});

Hooks::before($userCreate, function(&$transaction) use ($webserviceInfos) {
  /* Delete user if it exists */
  $client = connectRestClient($webserviceInfos['endPoint'], $webserviceInfos['user'], $webserviceInfos['password']);
  callRestMethod(
    $client,
    'DELETE',
    '/objects/user/uid=jsmith,ou=people,'.$webserviceInfos['ldapBase']
  );

  $requestBody = [
    'attrs' => [
      'user'  => [
        'givenName' => 'John',
        'sn' => 'Smith',
        'uid' => 'jsmith',
        'userPassword' => 'jsmith',
      ],
      'personalInfo' => [
        'personalTitle' => 'Dr'
      ]
    ]
  ];

  $transaction->request->body = json_encode($requestBody);
});

Hooks::beforeEach(function(&$transaction) use (&$stash, $webserviceInfos) {
  $replacements = [
    /* Fix base */
    urlencode('dc=example,dc=com') => urlencode($webserviceInfos['ldapBase']),
    /* Fix attrs attribute syntax */
    'attrs=sn,1,title,'.urlencode('*') => 'attrs[sn]=1&attrs[title]='.urlencode('*'),
    /* Fix templates option value */
    '&templates=true' => '',
  ];
  foreach ($replacements as $search => $replace) {
    $transaction->fullPath = str_replace($search, $replace, $transaction->fullPath);
    if (isset($transaction->request->uri)) {
      $transaction->request->uri = str_replace($search, $replace, $transaction->request->uri);
    }
  }

  if (isset($stash['token']) && ($transaction->expected->statusCode != '401')) {
    $transaction->request->headers->{'Session-Token'} = $stash['token'];
  }

  if ($transaction->request->method == 'DELETE') {
    /* Set up data for deletion */
    $client = connectRestClient($webserviceInfos['endPoint'], $webserviceInfos['user'], $webserviceInfos['password']);

    $result = callRestMethod(
      $client,
      'PATCH',
      '/objects/user/uid=jsmith,ou=people,'.$webserviceInfos['ldapBase'],
      [
        'user'  => [
          'givenName' => 'John',
          'sn' => 'Smith',
        ],
        'personalInfo' => [
          'personalTitle' => 'Dr'
        ]
      ]
    );

    if ($result->info->http_code == 404) {
      /* Create user if needed */
      $result = callRestMethod(
        $client,
        'POST',
        '/objects/user',
        [
          'attrs' => [
            'user'  => [
              'givenName' => 'John',
              'sn' => 'Smith',
              'uid' => 'jsmith',
              'userPassword' => 'jsmith',
            ],
            'personalInfo' => [
              'personalTitle' => 'Dr'
            ]
          ]
        ]
      );
    }

    if ($result->info->http_code >= 300) {
      /* Hack to see the error in the output */
      $transaction->request->body = $result->response;
    }
  }
});

Hooks::beforeAll(function(&$transactions) {
});

Hooks::beforeEachValidation(function(&$transaction) {
});

Hooks::afterEach(function(&$transaction) {
});

Hooks::afterAll(function(&$transaction) {
});
