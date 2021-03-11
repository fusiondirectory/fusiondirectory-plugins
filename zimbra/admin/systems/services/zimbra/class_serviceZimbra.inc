<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016-2021  FusionDirectory

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

class serviceZimbra extends simpleMailMethodService
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Zimbra'),
      'plDescription'   => _('Zimbra'),
      'plIcon'          => 'geticon.php?context=applications&icon=zimbra&size=16',
      'plObjectClass'   => ['fdZimbraServer'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return  [
      'main' => [
        'name'  => _('Settings'),
        'class' => ['fullwidth'],
        'attrs' => [
          new URLAttribute(
            _('URI'), _('URI of the Zimbra server soap service, with port, usually 7071'),
            'fdZimbraServerUri', TRUE
          ),
          new StringAttribute(
            _('User Agent'), _('User Agent to use to contact the Zimbra API'),
            'fdZimbraServerUserAgent', TRUE,
            'FusionDirectory'
          ),
          new SelectAttribute(
            _('Mailbox deletion'), _('What to do with the Zimbra account when mail tab is deactivated or user is deleted'),
            'fdZimbraServerDeletionType', TRUE,
            [  'delete',   'disable'], '',
            [_('Delete'),_('Disable')]
          ),
          new BooleanAttribute(
            _('Password synchronization'), _('Should the password be synchronized to the Zimbra instance'),
            'fdZimbraServerPasswordSync', FALSE,
            TRUE
          ),
          new StringAttribute(
            _('Login'), _('Login to use to contact the Zimbra API'),
            'fdZimbraServerLogin', TRUE
          ),
          new PasswordAttribute(
            _('Password'), _('Password to use to contact the Zimbra API'),
            'fdZimbraServerPassword', TRUE
          ),
        ]
      ],
      'domains' => [
        'template'  => get_template_path('zimbra_domains.tpl', TRUE, dirname(__FILE__)),
        'name'      => _('Domains'),
        'class'     => ['fullwidth'],
        'attrs'     => [
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('Domains handled by this Zimbra server'),
              'fdZimbraServerMailDomain',
              [
                new StringAttribute(
                  _('Domain'), _('Domain handled by this server'),
                  'zimbraDomain', TRUE
                ),
                new StringAttribute(
                  _('Classes of service'), _('Possible classes of service for this domain and their ids, separated by commas. Format is cos1Name|cos1Id,cos2Name|cos2Id.'),
                  'zimbraDomainCOS', TRUE,
                  '', '',
                  '/^[^,|]+\|[^,|]+(,[^,|]+\|[^,|]+)*$/',
                  'cos1|cos1id,cos2|cos2id'
                )
              ],
              ':',
              '',
              _('Domains')
            ),
            FALSE, [], TRUE
          )
        ]
      ]
    ];
  }

  function __construct ($dn = NULL, $parent = NULL)
  {
    parent::__construct($dn, $parent);
    $this->attributesAccess['fdZimbraServerMailDomain']->setLinearRendering(FALSE);
  }

  public function update (): bool
  {
    parent::update();

    if (($this->fdZimbraServerUri != '') && !preg_match('/:\d+$/', $this->fdZimbraServerUri)) {
      $this->fdZimbraServerUri = $this->fdZimbraServerUri . ':7071';
    }

    return TRUE;
  }
}
