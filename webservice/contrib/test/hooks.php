<?php

use Dredd\Hooks;

$stash = [];

$successfullLogin = '/login > Login to the API > 200 > application/json';

Hooks::before($successfullLogin, function(&$transaction) {
  $requestBody = $transaction->request->body;

  $requestBody['user']      = 'fd-admin';
  $requestBody['password']  = 'adminpwd';

  $transaction->request->body = json_encode($requestBody);
});

Hooks::after($successfullLogin, function(&$transaction) use (&$stash) {
  $parsedBody     = json_decode($transaction->real->body);
  $stash['token'] = $parsedBody;
});

Hooks::beforeEach(function(&$transaction) use (&$stash) {
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
