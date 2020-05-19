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
      'plObjectType'  => ['user'],

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
              'krbPrincipalName', 'This is the principal name in the RFC 1964 specified format',
              'krbPrincipalName', FALSE
            )
          ),
          new StringAttribute(
            'krbCanonicalName', 'If there are multiple krbPrincipalName values for an entry, this is the canonical principal name in the RFC 1964 specified format.',
            'krbCanonicalName', FALSE
          ),
          new BooleanAttribute(
            'krbUPEnabled', 'This flag is used to find whether directory User Password has to be used as kerberos password',
            'krbUPEnabled', FALSE
          ),
          new SetAttribute(
            new StringAttribute(
              'krbPrincipalKey', 'This attribute holds the principal\'s key (krbPrincipalKey) that is encrypted with the master key (krbMKey). The attribute is ASN.1 encoded.',
              'krbPrincipalKey', FALSE
            )
          ),
          new StringAttribute(
            'krbTicketPolicyReference', 'FDN pointing to a Kerberos Ticket Policy object.',
            'krbTicketPolicyReference', FALSE
          ),
          new StringAttribute(
            'krbPrincipalExpiration', 'The time at which the principal expires',
            'krbPrincipalExpiration', FALSE
          ),
          new StringAttribute(
            'krbPasswordExpiration', 'The time at which the principal\'s password expires',
            'krbPasswordExpiration', FALSE
          ),
          new StringAttribute(
            'krbPwdPolicyReference', 'FDN pointing to a Kerberos Password Policy object',
            'krbPwdPolicyReference', FALSE
          ),
          new IntAttribute(
            'krbPrincipalType', 'This specifies the type of the principal, the types could be any of the types mentioned in section 6.2 of RFC 4120',
            'krbPrincipalType', FALSE,
            FALSE, FALSE
          ),
          new SetAttribute(
            new StringAttribute(
              'krbPwdHistory', 'This attribute holds the principal\'s old keys (krbPwdHistory) that is encrypted with the kadmin/history key. The attribute is ASN.1 encoded.',
              'krbPwdHistory', FALSE
            )
          ),
          new StringAttribute(
            'krbLastPwdChange', 'The time at which the principal\'s password last password change happened.',
            'krbLastPwdChange', FALSE
          ),
          new StringAttribute(
            'krbLastAdminUnlock', 'The time at which the principal was last administratively unlocked.',
            'krbLastAdminUnlock', FALSE
          ),
          new SetAttribute(
            new StringAttribute(
              'krbPrincipalAliases', 'This stores the alternate principal names for the principal in the RFC 1964 specified format',
              'krbPrincipalAliases', FALSE
            )
          ),
          new StringAttribute(
            'krbLastSuccessfulAuth', 'The time at which the principal\'s last successful authentication happened.',
            'krbLastSuccessfulAuth', FALSE
          ),
          new StringAttribute(
            'krbLastFailedAuth', 'The time at which the principal\'s last failed authentication happened.',
            'krbLastFailedAuth', FALSE
          ),
          new IntAttribute(
            'krbLoginFailedCount', 'This attribute stores the number of failed authentication attempts happened for the principal since the last successful authentication.',
            'krbLoginFailedCount', FALSE,
            FALSE, FALSE
          ),
          new SetAttribute(
            new StringAttribute(
              'krbExtraData', 'This attribute holds the application specific data.',
              'krbExtraData', FALSE
            )
          ),
          new SetAttribute(
            new StringAttribute(
              'krbAllowedToDelegateTo', 'A list of services to which a service principal can delegate.',
              'krbAllowedToDelegateTo', FALSE
            )
          ),
          new SetAttribute(
            new StringAttribute(
              'krbPrincipalAuthInd', 'A list of authentication indicator strings, one of which must be satisfied to authenticate to the principal as a service.',
              'krbPrincipalAuthInd', FALSE
            )
          ),
        ],
      ],
      'krbTicketPolicyAux' => [
        'name'  => _('krbTicketPolicyAux'),
        'attrs' => [
          new IntAttribute(
            'krbTicketFlags', 'The krbTicketFlags attribute holds information about the kerberos flags for a principal',
            'krbTicketFlags', FALSE,
            FALSE, FALSE
          ),
          new IntAttribute(
            'krbMaxTicketLife', 'The maximum ticket lifetime for a principal in seconds',
            'krbMaxTicketLife', FALSE,
            FALSE, FALSE
          ),
          new IntAttribute(
            'krbMaxRenewableAge', 'Maximum renewable lifetime for a principal\'s ticket in seconds',
            'krbMaxRenewableAge', FALSE,
            FALSE, FALSE
          ),
        ],
      ],
    ];
  }
}
