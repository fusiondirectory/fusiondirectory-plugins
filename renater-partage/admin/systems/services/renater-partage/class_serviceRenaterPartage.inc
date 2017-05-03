<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016-2017  FusionDirectory

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

class serviceRenaterPartage extends simpleService
{
  var $objectclasses = array('fdRenaterPartageServer');

  static function plInfo()
  {
    return array(
      'plShortName'     => _('Renater Partage'),
      'plDescription'   => _('Renater Partage'),
      'plIcon'          => 'geticon.php?context=applications&icon=renater-partage&size=16',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array (
      'main' => array (
        'name'  => _('Settings'),
        'class' => array('fullwidth'),
        'attrs' => array (
          new URLAttribute(
            _('URI'), _('URI to contact the Renater Partage API'),
            'fdRenaterPartageServerUri', TRUE
          ),
          new StringAttribute(
            _('User Agent'), _('User Agent to use to contact the Renater Partage API'),
            'fdRenaterPartageServerUserAgent', TRUE,
            'FusionDirectory'
          ),
          new SelectAttribute(
            _('Mailbox deletion'), _('What to do with the PARTAGE account when mail tab is deactivated or user is deleted'),
            'fdRenaterPartageServerDeletionType', TRUE,
            array(  'delete',   'disable'), '',
            array(_('Delete'),_('Disable'))
          ),
        )
      ),
      'domains' => array(
        'template'  => get_template_path('renater_domains.tpl', TRUE, dirname(__FILE__)),
        'name'      => _('Domains'),
        'class'     => array('fullwidth'),
        'attrs'     => array(
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('Domains handled by this Renater Partage server'),
              'fdRenaterPartageServerMailDomain',
              array(
                new StringAttribute (
                  _('Domain'), _('Domain handled by this server'),
                  'renaterPartageDomain', TRUE
                ),
                new StringAttribute (
                  _('Key'), _('Key for this domain'),
                  'renaterPartageDomainKey', TRUE
                ),
                new StringAttribute (
                  _('Class of service'), _('Possible classes of service for this domain, separated by commas'),
                  'renaterPartageDomainCOS', TRUE
                )
              ),
              ':',
              '',
              _('Domains')
            ),
            FALSE, array(), TRUE
          )
        )
      )
    );
  }

  function __construct ($dn = NULL, $parent = NULL)
  {
    parent::__construct($dn, $parent);
    $this->attributesAccess['fdRenaterPartageServerMailDomain']->setLinearRendering(FALSE);
  }

  protected function post_save()
  {
    parent::post_save();
    mailMethod::resetMailServersCache();
  }

  protected function post_remove ()
  {
    parent::post_remove();
    mailMethod::resetMailServersCache();
  }
}
?>
