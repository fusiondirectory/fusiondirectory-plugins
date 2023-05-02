<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2015-2020  FusionDirectory

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

/*
   h= Acceptable hash algorithms (plain-text; OPTIONAL, defaults to
      allowing all algorithms).  A colon-separated list of hash
      algorithms that might be used.  Unrecognized algorithms MUST be
      ignored.  Refer to Section 3.3 for a discussion of the hash
      algorithms implemented by Signers and Verifiers.  The set of
      algorithms listed in this tag in each record is an operational
      choice made by the Signer.

      ABNF:

      key-h-tag       = %x68 [FWS] "=" [FWS] key-h-tag-alg
                        *( [FWS] ":" [FWS] key-h-tag-alg )
      key-h-tag-alg   = "sha1" / "sha256" / x-key-h-tag-alg
      x-key-h-tag-alg = hyphenated-word   ; for future extension

   k= Key type (plain-text; OPTIONAL, default is "rsa").  Signers and
      Verifiers MUST support the "rsa" key type.  The "rsa" key type
      indicates that an ASN.1 DER-encoded [ITU-X660-1997] RSAPublicKey
      (see [RFC3447], Sections 3.1 and A.1.1) is being used in the "p="
      tag.  (Note: the "p=" tag further encodes the value using the
      base64 algorithm.)  Unrecognized key types MUST be ignored.

      ABNF:

      key-k-tag        = %x76 [FWS] "=" [FWS] key-k-tag-type
      key-k-tag-type   = "rsa" / x-key-k-tag-type
      x-key-k-tag-type = hyphenated-word   ; for future extension

   n= Notes that might be of interest to a human (qp-section; OPTIONAL,
      default is empty).  No interpretation is made by any program.
      This tag should be used sparingly in any key server mechanism that
      has space limitations (notably DNS).  This is intended for use by
      administrators, not end users.

      ABNF:

      key-n-tag    = %x6e [FWS] "=" [FWS] qp-section

   p= Public-key data (base64; REQUIRED).  An empty value means that
      this public key has been revoked.  The syntax and semantics of
      this tag value before being encoded in base64 are defined by the
      "k=" tag.

         INFORMATIVE RATIONALE: If a private key has been compromised or
         otherwise disabled (e.g., an outsourcing contract has been
         terminated), a Signer might want to explicitly state that it
         knows about the selector, but all messages using that selector
         should fail verification.  Verifiers SHOULD return an error
         code for any DKIM-Signature header field with a selector
         referencing a revoked key.  (See Section 6.1.2 for details.)

      ABNF:

      key-p-tag    = %x70 [FWS] "=" [ [FWS] base64string]

         INFORMATIVE NOTE: A base64string is permitted to include
         whitespace (FWS) at arbitrary places; however, any CRLFs must
         be followed by at least one WSP character.  Implementers and
         administrators are cautioned to ensure that selector TXT RRs
         conform to this specification.

   s= Service Type (plain-text; OPTIONAL; default is "*").  A colon-
      separated list of service types to which this record applies.
      Verifiers for a given service type MUST ignore this record if the
      appropriate type is not listed.  Unrecognized service types MUST
      be ignored.  Currently defined service types are as follows:

      *  matches all service types

      email   electronic mail (not necessarily limited to SMTP)

      This tag is intended to constrain the use of keys for other
      purposes, should use of DKIM be defined by other services in the
      future.

      ABNF:

      key-s-tag        = %x73 [FWS] "=" [FWS] key-s-tag-type
                         *( [FWS] ":" [FWS] key-s-tag-type )
      key-s-tag-type   = "email" / "*" / x-key-s-tag-type
      x-key-s-tag-type = hyphenated-word   ; for future extension

   t= Flags, represented as a colon-separated list of names (plain-
      text; OPTIONAL, default is no flags set).  Unrecognized flags MUST
      be ignored.  The defined flags are as follows:

      y  This domain is testing DKIM.  Verifiers MUST NOT treat messages
         from Signers in testing mode differently from unsigned email,
         even should the signature fail to verify.  Verifiers MAY wish
         to track testing mode results to assist the Signer.

      s  Any DKIM-Signature header fields using the "i=" tag MUST have
         the same domain value on the right-hand side of the "@" in the
         "i=" tag and the value of the "d=" tag.  That is, the "i="
         domain MUST NOT be a subdomain of "d=".  Use of this flag is
         RECOMMENDED unless subdomaining is required.
*/

class DkimRecordAttribute extends CompositeAttribute
{
  public function __construct (string $description, string $ldapName, string $acl = '')
  {
    $attributes = [
      'h' => new StringAttribute(
        _('Hash algorithms'),
        _('A colon-separated list of hash algorithms that might be used, defaults to allowing all algorithms.'),
        $ldapName.'_h', FALSE
      ),
      'k' => new StringAttribute(
        _('Key type'),
        _('Key type of the public key, default is "rsa".'),
        $ldapName.'_k', FALSE
      ),
      'n' => new StringAttribute(
        _('Notes'),
        _('Notes that might be of interest to a human. No interpretation is made by any program.'),
        $ldapName.'_n', FALSE
      ),
      'p' => new TextAreaAttribute(
        _('Public key'),
        _('Public-key data encoded in base64. An empty value means that this public key has been revoked.'),
        $ldapName.'_p', FALSE
      ),
      's' => new StringAttribute(
        _('Service type'),
        _('A colon-separated list of service types to which this record applies, default is "*".'),
        $ldapName.'_s', FALSE
      ),
      't' => new StringAttribute(
        _('Flags'),
        _('Flags, represented as a colon-separated list of names, default is no flags set.'),
        $ldapName.'_t', FALSE
      ),
    ];
    parent::__construct($description, $ldapName, $attributes, FALSE, FALSE, $acl, NULL);
  }

  function readValues (string $value): array
  {
    /* From RFC 6376

       Formally, the ABNF syntax rules are as follows:

       tag-list  =  tag-spec *( ";" tag-spec ) [ ";" ]
       tag-spec  =  [FWS] tag-name [FWS] "=" [FWS] tag-value [FWS]
       tag-name  =  ALPHA *ALNUMPUNC
       tag-value =  [ tval *( 1*(WSP / FWS) tval ) ]
                         ; Prohibits WSP and FWS at beginning and end
       tval      =  1*VALCHAR
       VALCHAR   =  %x21-3A / %x3C-7E
                         ; EXCLAMATION to TILDE except SEMICOLON
       ALNUMPUNC =  ALPHA / DIGIT / "_"
    */
    $parts  = explode(';', $value);
    $tags   = [];
    foreach ($parts as $part) {
      if (empty($part)) {
        continue;
      }
      list($tagname, $tagvalue) = explode('=', $part, 2);
      $tagname  = trim($tagname, " \t\n\r");
      $tagvalue = trim($tagvalue, " \t\n\r");
      $tags[$tagname] = $tagvalue;
    }
    $values = [];
    foreach (array_keys($this->attributes) as $name) {
      if (isset($tags[$name])) {
        $values[] = $tags[$name];
      } else {
        $values[] = '';
      }
    }
    return $values;
  }

  function writeValues (array $values)
  {
    $values = array_filter(
      $values,
      function ($value)
      {
        return ($value !== '');
      }
    );
    if (!isset($values['p'])) {
      /* Special case: empty public key means revoked key */
      $values['p'] = '';
    }
    /* Remove newlines in public key data */
    $values['p'] = str_replace(["\n", "\r"], '', $values['p']);
    return implode(
      '; ',
      array_map(
        function ($key, $value) {
          return $key.'='.$value;
        },
        array_keys($values),
        $values
      )
    );
  }

  function check ()
  {
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    }
    $p = str_replace(["\n", "\r"], '', $this->attributes['p']->getValue());
    if (!empty($p) && (base64_decode($p, TRUE) === FALSE)) {
      return new SimplePluginCheckError(
        $this,
        _('The "Public key" field does not contain valid base64 data')
      );
    }
    return $error;
  }
}
