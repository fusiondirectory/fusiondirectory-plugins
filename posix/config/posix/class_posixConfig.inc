<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2015-2017 FusionDirectory

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

class posixConfig extends simplePlugin
{
  var $objectclasses = array();

  static function plInfo()
  {
    return array(
      'plShortName'     => _('POSIX configuration'),
      'plDescription'   => _('FusionDirectory POSIX plugin configuration'),
      'plCategory'      => array('configuration'),
      'plObjectType'    => array('smallConfig'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Posix'),
        'class' => array('critical'),
        'attrs' => array(
          new StringAttribute (
            _('POSIX groups RDN'), _('The branch where POSIX groups are stored.'),
            'fdGroupRDN', TRUE,
            'ou=groups'
          ),
          new IntAttribute (
            _('Group/user min id'),
            _('The minimum assignable user or group id to avoid security leaks with id 0 accounts.'),
            'fdMinId', FALSE,
            0, FALSE, 100
          ),
          new StringAttribute (
            _('Next id hook'), _('A script to be called for finding the next free id number for users or groups.'),
            'fdNextIdHook', FALSE
          ),
          new IntAttribute (
            _('Base number for user id'),
            _('Where to start looking for a new free user id.'),
            'fdUidNumberBase', FALSE,
            0, FALSE, 1100
          ),
          new IntAttribute (
            _('Base number for group id'),
            _('Where to start looking for a new free group id.'),
            'fdGidNumberBase', FALSE,
            0, FALSE, 1100
          ),
          new SelectAttribute (
            _('Id allocation method'), _('Method to allocate user/group ids'),
            'fdIdAllocationMethod', TRUE,
            array('traditional', 'pool'), 'traditional',
            array(_('Traditional'), _('Samba unix id pool'))
          ),
          new IntAttribute (
            _('Pool user id min'), _('Minimum value for user id when using pool method'),
            'fdUidNumberPoolMin', FALSE,
            0, FALSE, 10000
          ),
          new IntAttribute (
            _('Pool user id max'), _('Maximum value for user id when using pool method'),
            'fdUidNumberPoolMax', FALSE,
            0, FALSE, 40000
          ),
          new IntAttribute (
            _('Pool group id min'), _('Minimum value for group id when using pool method'),
            'fdGidNumberPoolMin', FALSE,
            0, FALSE, 10000
          ),
          new IntAttribute (
            _('Pool group id max'), _('Maximum value for group id when using pool method'),
            'fdGidNumberPoolMax', FALSE,
            0, FALSE, 40000
          ),
        )
      ),
      'shells' => array(
        'name'  => _('Shells'),
        'attrs' => array(
          new SetAttribute(
            new StringAttribute (
              _('Available shells'), _('Available POSIX shells for FD users.'),
              'fdShells', FALSE
            ),
            array('/bin/ash','/bin/bash','/bin/csh','/bin/sh','/bin/ksh',
                  '/bin/tcsh','/bin/dash','/bin/zsh','/sbin/nologin', '/bin/false', '/usr/bin/git-shell')
          ),
          new SelectAttribute (
            _('Default shell'), _('Shell used by default when activating Unix tab.'),
            'fdDefaultShell', TRUE
          ),
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE, $attributesInfo = NULL)
  {
    parent::__construct($dn, $object, $parent, $mainTab, $attributesInfo);

    $this->attributesAccess['fdDefaultShell']->setChoices($this->fdShells);

    $this->attributesAccess['fdIdAllocationMethod']->setManagedAttributes(
      array(
        'erase' => array (
          'traditional' => array (
            'fdUidNumberPoolMin','fdUidNumberPoolMax',
            'fdGidNumberPoolMin','fdGidNumberPoolMax',
          )
        )
      )
    );
  }

  function save_object()
  {
    parent::save_object();
    $this->attributesAccess['fdDefaultShell']->setChoices($this->fdShells);
  }
}
?>
