/*
  Copyright (C) 2018 Lukas Buchs
  Copyright (C) 2019 FusionDirectory
  license https://github.com/lbuchs/WebAuthn/blob/master/LICENSE MIT
*/

/**
 * creates a new FIDO2 registration
 * @returns {undefined}
 */
function webauthnNewRegistration()
{
  if (!window.fetch || !navigator.credentials || !navigator.credentials.create) {
    window.alert('Browser not supported.');
    return;
  }

  // get default args
  window.fetch(window.location.href + '&webauthn=getCreateArgs', {method:'GET',cache:'no-cache'}).then(function(response) {
    return response.json();
  }).then(function(json) {
    // convert base64 to arraybuffer
    if (json.success === false) {
      // error handling
      throw new Error(json.msg);
    }

    // replace binary base64 data with ArrayBuffer. a other way to do this
    // is the reviver function of JSON.parse()
    recursiveBase64StrToArrayBuffer(json);
    return json;
  }).then(function(createCredentialArgs) {
    // create credentials
    console.log(createCredentialArgs);
    return navigator.credentials.create(createCredentialArgs);
  }).then(function(cred) {
    // convert to base64
    return {
      clientDataJSON: cred.response.clientDataJSON  ? arrayBufferToBase64(cred.response.clientDataJSON) : null,
      attestationObject: cred.response.attestationObject ? arrayBufferToBase64(cred.response.attestationObject) : null
    };
  }).then(JSON.stringify).then(function(AuthenticatorAttestationResponse) {
    // transfer to server
    return window.fetch(window.location.href + '&webauthn=processCreate', {method:'POST', body: AuthenticatorAttestationResponse, cache:'no-cache'})
  }).then(function(response) {
    // convert to JSON
    return response.json();
  }).then(function(json) {
    // analyze response
    if (json.success) {
      window.alert(json.msg || 'registration success');
      document.mainform.submit();
    } else {
      throw new Error(json.msg);
    }
  }).catch(function(err) {
    // catch errors
    window.alert(err.message || 'unknown error occured');
  });
}


/**
 * checks a FIDO2 registration
 * @returns {undefined}
 */
function webauthnCheckRegistration()
{
  if (!window.fetch || !navigator.credentials || !navigator.credentials.create) {
    window.alert('Browser not supported.');
    return;
  }

  // get default args
  window.fetch(window.location.href + '?webauthn=getGetArgs', {method:'GET',cache:'no-cache'}).then(function(response) {
      return response.json();

  }).then(function(json) {
    // convert base64 to arraybuffer

    if (json.success === false) {
      // error handling
      throw new Error(json.msg);
    }

    // replace binary base64 data with ArrayBuffer. a other way to do this
    // is the reviver function of JSON.parse()
    recursiveBase64StrToArrayBuffer(json);
    return json;
  }).then(function(getCredentialArgs) {
    // create credentials
    return navigator.credentials.get(getCredentialArgs);

  }).then(function(cred) {
    // convert to base64
    return {
      id: cred.rawId ? arrayBufferToBase64(cred.rawId) : null,
      clientDataJSON: cred.response.clientDataJSON  ? arrayBufferToBase64(cred.response.clientDataJSON) : null,
      authenticatorData: cred.response.authenticatorData ? arrayBufferToBase64(cred.response.authenticatorData) : null,
      signature : cred.response.signature ? arrayBufferToBase64(cred.response.signature) : null
    };
  }).then(JSON.stringify).then(function(AuthenticatorAttestationResponse) {
    // transfer to server
    return window.fetch(window.location.href + '?webauthn=processGet', {method:'POST', body: AuthenticatorAttestationResponse, cache:'no-cache'});

  }).then(function(response) {
    // convert to json
    return response.json();
  }).then(function(json) {
    // analyze response
    if (json.success) {
      document.location = 'main.php';
    } else {
      throw new Error(json.msg);
    }

  }).catch(function(err) {
    // catch errors
    window.alert(err.message || 'unknown error occured');
  });
}

/**
 * convert RFC 1342-like base64 strings to array buffer
 * @param {mixed} obj
 * @returns {undefined}
 */
function recursiveBase64StrToArrayBuffer(obj)
{
  let prefix = '?BINARY?B?';
  let suffix = '?=';
  if (typeof obj === 'object') {
    for (let key in obj) {
      if (typeof obj[key] === 'string') {
        let str = obj[key];
        if (str.substring(0, prefix.length) === prefix && str.substring(str.length - suffix.length) === suffix) {
          str = str.substring(prefix.length, str.length - suffix.length);

          let binary_string = window.atob(str);
          let len = binary_string.length;
          let bytes = new Uint8Array(len);
          for (var i = 0; i < len; i++)        {
            bytes[i] = binary_string.charCodeAt(i);
          }
          obj[key] = bytes.buffer;
        }
      } else {
        recursiveBase64StrToArrayBuffer(obj[key]);
      }
    }
  }
}

/**
 * Convert a ArrayBuffer to Base64
 * @param {ArrayBuffer} buffer
 * @returns {String}
 */
function arrayBufferToBase64(buffer) {
  var binary = '';
  var bytes = new Uint8Array(buffer);
  var len = bytes.byteLength;
  for (var i = 0; i < len; i++) {
    binary += String.fromCharCode( bytes[ i ] );
  }
  return window.btoa(binary);
}

/**
 * force https on load
 * @returns {undefined}
 */
window.onload = function() {
  if (location.protocol !== 'https:' && location.host !== 'localhost') {
    location.href = location.href.replace('http', 'https');
  }
}

