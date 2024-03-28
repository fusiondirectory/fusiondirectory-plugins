<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2019-2020 FusionDirectory

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*/

class KrbTicketFlagsAttribute extends CompositeAttribute
{

  function __construct ($ldapName, $attributes, $acl = "")
  {
    parent::__construct( _('The krbTicketFlags attribute holds information about the kerberos flags for a principal'),$ldapName, $attributes, '', '', $acl);
  }

  function readValues(string $value): array
  {
    $values = array();
    foreach($this->attributes as $attribute){
        if ( ($value & $attribute->trueValue) > 0 )
             $values[] = $attribute->trueValue;
        else $values[] = 0;
    }
    return $values;
  }

  function writeValues(array $values): string
  {
    $flag = 0;
    foreach ($values as $value) {
      $flag += $value;
    }

    if ($flag>0) return $flag;
    else return "";
  }
}

// MIT kerberos requires generalizedtime having suffix zeros not beeing stripped
// class LdapGeneralizedTime strips zeros of hour, minute, seconds
class KrbDateAttribute extends DateAttribute
{
  function __construct ($label, $description, $ldapName, $required, $defaultValue = 'now', $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, '', $defaultValue, $acl);
  }

  protected function dateToLdap($dateValue)
  {
    return $dateValue->format('YmdHis') . "Z";
  }

}

class kerberosAccount extends simplePlugin
{
  protected $displayHeader = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Kerberos'),
      'plDescription' => _('Kerberos tab'),
      'plIcon'        => 'geticon.php?context=applications&icon=kerberos&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=kerberos&size=16',
      'plSelfModify'  => FALSE,
      'plObjectClass' => ['krbPrincipalAux','krbTicketPolicyAux'],
      'plFilter'      => '(objectClass=krbPrincipalAux)',
      'plObjectType'  => ['user', 'server','workstation','terminal','printer','component','phone','mobilePhone'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'krbPrincipalAux' => [
        'name'  => _('krbPrincipalAux'),
        'attrs' => [
          new SetAttribute(
            new StringAttribute(
              _('krbPrincipalName'), _('This is the principal name in the RFC 1964 specified format'),
              'krbPrincipalName', TRUE
            )
          ),
          new StringAttribute(
            _('krbCanonicalName'), _('If there are multiple krbPrincipalName values for an entry, this is the canonical principal name in the RFC 1964 specified format.'),
            'krbCanonicalName', FALSE
          ),
          new BooleanAttribute(
            _('krbUPEnabled'), _('This flag is used to find whether directory User Password has to be used as kerberos password'),
            'krbUPEnabled', FALSE
          ),
          new HiddenArrayAttribute(
            'krbPrincipalKey', FALSE
            // This attribute holds the principal's key (krbPrincipalKey) that is encrypted with the master key (krbMKey). The attribute is ASN.1 encoded.
          ),
          new StringAttribute(
            _('krbTicketPolicyReference'), _('FDN pointing to a Kerberos Ticket Policy object.'),
            'krbTicketPolicyReference', FALSE
          ),
          new KrbDateAttribute(
            _('krbPrincipalExpiration'), _('The time at which the principal expires'),
            'krbPrincipalExpiration', FALSE, ''
          ),
          new KrbDateAttribute(
            _('krbPasswordExpiration'), _('The time at which the principal\'s password expires'),
            'krbPasswordExpiration', FALSE, ''
          ),
          new StringAttribute(
            _('krbPwdPolicyReference'), _('FDN pointing to a Kerberos Password Policy object'),
            'krbPwdPolicyReference', FALSE
          ),
          new IntAttribute(
            _('krbPrincipalType'), _('This specifies the type of the principal, the types could be any of the types mentioned in section 6.2 of RFC 4120'),
            'krbPrincipalType', FALSE,
            FALSE, FALSE
          ),
          new SetAttribute(
            new StringAttribute(
              _('krbPwdHistory'), _('This attribute holds the principal\'s old keys (krbPwdHistory) that is encrypted with the kadmin/history key. The attribute is ASN.1 encoded.'),
              'krbPwdHistory', FALSE
            )
          ),
          new ReadOnlyLDAPAttribute(
            _('krbLastPwdChange'), _('The time at which the principal\'s password last password change happened.'),
            'krbLastPwdChange', FALSE
          ),
          new ReadOnlyLDAPAttribute(
            _('krbLastAdminUnlock'), _('The time at which the principal was last administratively unlocked.'),
            'krbLastAdminUnlock', FALSE
          ),
          new SetAttribute(
            new StringAttribute(
              _('krbPrincipalAliases'), _('This stores the alternate principal names for the principal in the RFC 1964 specified format'),
              'krbPrincipalAliases', FALSE
            )
          ),
          new ReadOnlyLDAPAttribute(
            _('krbLastSuccessfulAuth'), _('The time at which the principal\'s last successful authentication happened.'),
            'krbLastSuccessfulAuth', FALSE
          ),
          new ReadOnlyLDAPAttribute(
            _('krbLastFailedAuth'), _('The time at which the principal\'s last failed authentication happened.'),
            'krbLastFailedAuth', FALSE
          ),
          new ReadOnlyLDAPAttribute(
            _('krbLoginFailedCount'), _('This attribute stores the number of failed authentication attempts happened for the principal since the last successful authentication.'),
            'krbLoginFailedCount', FALSE,
          ),
          new NoAddOrderedArrayAttribute(
            new KrbExtraDataAttribute(
              _('krbExtraData'), _('This attribute holds the application specific data.'),
              'krbExtraData', FALSE
            ),
            FALSE,
            array( // default values: one attribute of 'last modified'
              KrbExtraDataAttribute::packLastModification()
            )
          ),
          new SetAttribute(
            new StringAttribute(
              _('krbAllowedToDelegateTo'), _('A list of services to which a service principal can delegate.'),
              'krbAllowedToDelegateTo', FALSE
            )
          ),
          new SetAttribute(
            new StringAttribute(
              _('krbPrincipalAuthInd'), _('A list of authentication indicator strings, one of which must be satisfied to authenticate to the principal as a service.'),
              'krbPrincipalAuthInd', FALSE
            )
          ),
        ],
      ],
       // flag description taken from http://web.mit.edu/kerberos/krb5-latest/doc/admin/conf_files/kdc_conf.html
      'krbTicketPolicyAux' => [
        'name'  => _('Ticket Flags and Timing'),
        'attrs' => [
          new KrbTicketFlagsAttribute(
            'krbTicketFlags',
            array(
                new BooleanAttribute (
                  _('Prohibit postdated tickets'), _('Prohibit the principal to obtain postdateable tickets.'),
                  'DISALLOW_POSTDATED', FALSE, FALSE, '',
                  0x00000001, 0
                ),
                new BooleanAttribute (
                  _('Prohibit forwardable tickets'), _('Prohibit the principal to obtain a session key for another user, permitting user-to-user authentication for this principal.'),
                  'DISALLOW_FORWARDABLE', FALSE, FALSE, '',
                  0x00000002, 0
                ),
                new BooleanAttribute (
                  _('Disallow Ticket-Granting Service'), _('Disallow a principal to obtain tickets based on a ticket-granting-ticket, rather than repeating the authentication process that was used to obtain the TGT.'),
                  'DISALLOW_TGT_BASED', FALSE, FALSE, '',
                  0x00000004, 0
                ),
                new BooleanAttribute (
                  _('Prohibit renewable tickets'), _('Prohibit the principal to obtain renewable tickets.'),
                  'DISALLOW_RENEWABLE', FALSE, FALSE, '',
                  0x00000008, 0
                ),
                new BooleanAttribute (
                  _('Disallow proxiable tickets'), _('Prohibit the principal to obtain proxy tickets.'),
                  'DISALLOW_PROXIABLE', FALSE, FALSE, '',
                  0x00000010, 0
                ),
                new BooleanAttribute (
                  _('Disallow user to user authentification'), _('Enabling this flag disallows the principal to obtain a session key for another user, permitting user-to-user authentication for this principal.'),
                  'DISALLOW_DUP_SKEY', FALSE, FALSE, '',
                  0x00000020, 0
                ),
                new BooleanAttribute (
                  _('Forbid ticket issuance'), _('Enabling this flag essentially deactivates the principal within this realm.'),
                  'DISALLOW_ALL_TIX', FALSE, FALSE, '',
                  0x00000040, 0
                ),
                new BooleanAttribute (
                  _('Preauthentication required'), _('If this flag is enabled on a client principal, then that principal is required to preauthenticate to the KDC before receiving any tickets. On a service principal, enabling this flag means that service tickets for this principal will only be issued to clients with a TGT that has the preauthenticated bit set.'),
                  'REQUIRES_PRE_AUTH', FALSE, TRUE, '',
                  0x00000080, 0
                ),
                new BooleanAttribute (
                  _('Hardware preauthentication'), _('If this flag is enabled, then the principal is required to preauthenticate using a hardware device before receiving any tickets.'),
                  'REQUIRES_HW_AUTH', FALSE, FALSE, '',
                  0x00000100, 0
                ),
                new BooleanAttribute (
                  _('Force a password change'), _('Enabling this flag forces a password change for this principal.'),
                  'REQUIRES_PWCHANGE', FALSE, FALSE, '',
                  0x00000200, 0
                ),
                new BooleanAttribute (
                  _('UNKNOWN'), _('Unknown Flag 0x00000400'),
                  'UNKNOWN_0x00000400', FALSE, FALSE, '',
                  0x00000400, 0
                ),
                new BooleanAttribute (
                  _('UNKNOWN'), _('Unknown Flag 0x00000800'),
                  'UNKNOWN_0x00000800', FALSE, FALSE, '',
                  0x00000800, 0
                ),
                new BooleanAttribute (
                  _('Prohibit issuance of service tickets'), _('Prohibit the KDC to issue service tickets for this principal.'),
                  'DISALLOW_SVR', FALSE, FALSE, '',
                  0x00001000, 0
                ),
                new BooleanAttribute (
                  _('Password change service'), _('If this flag is enabled, it marks this principal as a password change service. This should only be used in special cases, for example, if a user’s password has expired, then the user has to get tickets for that principal without going through the normal password authentication in order to be able to change the password.'),
                  'PWCHANGE_SERVICE', FALSE, FALSE, '',
                  0x00002000, 0
                ),
                new BooleanAttribute (
                  _('SUPPORT_DESMD5'), _('SUPPORT_DESMD5'),
                  'SUPPORT_DESMD5', FALSE, FALSE, '',
                  0x00004000, 0
                ),
                new BooleanAttribute (
                  _('NEW_PRINC'), _('NEW_PRINC'),
                  'NEW_PRINC', FALSE, FALSE, '',
                  0x00008000, 0
                ),
              )
          ),
          new TimeAttribute(
            _('Maximum ticket lifetime'), _('The maximum ticket lifetime for a principal in seconds'),
            'krbMaxTicketLife', FALSE,
            FALSE, FALSE
          ),
          new TimeAttribute(
            _('Maximum renewable lifetime'), _('Maximum renewable lifetime for a principal\'s ticket in seconds'),
            'krbMaxRenewableAge', FALSE,
            FALSE, FALSE
          ),
        ],
      ],
      'invisible' => [
        'class' => array('invisible'),
      ],
    ];
  }

  function prettyPrintDateTime($attribute) {
    if ( isset($this->attrs[$attribute]) && ($this->attrs[$attribute][0]!='')) {
      $date = LdapGeneralizedTime::fromString($this->attrs[$attribute][0]);
      $date->setTimezone(timezone::getDefaultTimeZone());
      $this->$attribute = $date->format('Y-m-d H:i:s');
    } else {
      $this->$attribute = _('Never');
    }
  }

  function __construct ($dn = NULL, $baseobject = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $baseobject, $parent, $mainTab);

    kerberosAccount::prettyPrintDateTime('krbLastPwdChange');
    kerberosAccount::prettyPrintDateTime('krbLastAdminUnlock');
    kerberosAccount::prettyPrintDateTime('krbLastSuccessfulAuth');
    kerberosAccount::prettyPrintDateTime('krbLastFailedAuth');
  }

  function prepare_save(): array
  {
    // check if any krb attribute has been changed
    $krbchanged=false;
    foreach ($this->attributesAccess as $key => $attr) {
        if (isset($attr)) {
            $krbchanged = $krbchanged | $attr->hasChanged();
        }
    }

    $errors = parent::prepare_save();

    if (!empty($errors)) {
      return $errors;
    }


    // update 'last modification' attribute data
    if ($krbchanged) {
      for ($i = 0; $i < count($this->attrs['krbExtraData']); $i++) {
          // find krbExtraData attribute which contains 'last modification' data
          if (KrbExtraDataAttribute::isLastModification($this->attrs['krbExtraData'][$i])) {
            $this->attrs['krbExtraData'][$i] = KrbExtraDataAttribute::packLastModification();
          }
      }
    }

    return $errors;
  }

}
