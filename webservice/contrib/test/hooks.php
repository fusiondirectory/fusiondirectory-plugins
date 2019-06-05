<?php

use Dredd\Hooks;

$stash = [];

$successfullLogin = '/login > Login to the API > 200 > application/json';
$logout           = '/logout > End the currently opened session > 204';
$userCreate       = '/objects/{type} > Create an object of specific type > 201 > application/json';
$ldapBase         = 'dc=fusiondirectory,dc=org';

Hooks::before($successfullLogin, function(&$transaction) {
  $requestBody = [];

  $requestBody['user']      = 'fd-admin';
  $requestBody['password']  = 'adminpwd';


  $transaction->request->body = json_encode($requestBody);
});

Hooks::after($successfullLogin, function(&$transaction) use (&$stash) {
  $parsedBody     = json_decode($transaction->real->body);
  $stash['token'] = $parsedBody;
});

Hooks::before($logout, function(&$transaction) {
  // Skip logout to avoid destroying the session
  $transaction->skip = TRUE;
});

Hooks::before($userCreate, function(&$transaction) {
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

Hooks::beforeEach(function(&$transaction) use (&$stash, $ldapBase) {
  $transaction->fullPath = str_replace(urlencode('dc=example,dc=com'), urlencode($ldapBase), $transaction->fullPath);
  if (isset($transaction->request->uri)) {
    $transaction->request->uri = str_replace(urlencode('dc=example,dc=com'), urlencode($ldapBase), $transaction->request->uri);
  }

  if (isset($stash['token']) && ($transaction->expected->statusCode != '401')) {
    $transaction->request->headers->{'Session-Token'} = $stash['token'];
  }
});

Hooks::beforeAll(function(&$transaction) {
});

Hooks::beforeEachValidation(function(&$transaction) {
});

Hooks::afterEach(function(&$transaction) {
});

Hooks::afterAll(function(&$transaction) {
});
