<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017-2018 FusionDirectory

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

class sinapsConfig extends simplePlugin
{
  var $objectclasses  = array('fdSinapsPluginConf');

  static function plInfo()
  {
    return array(
      'plShortName'     => _('SINAPS configuration'),
      'plDescription'   => _('FusionDirectory SINAPS plugin configuration'),
      'plCategory'      => array('configuration'),
      'plObjectType'    => array('smallConfig'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    global $config;

    return array(
      'main' => array(
        'name'  => _('Sinaps'),
        'attrs' => array(
          new BooleanAttribute (
            _('Enable SINAPS integration'), _('Whether to enable the SINAPS integration'),
            'fdSinapsEnabled', FALSE,
            TRUE
          ),
          new BooleanAttribute (
            _('Dry run mode'), _('Do not insert data in FusionDirectory, dump it to a file'),
            'fdSinapsDryRun', FALSE,
            FALSE
          ),
          new StringAttribute (
            _('Acquisiton URL'), _('Full URL to which acquisiton events should be sent'),
            'fdSinapsAcquisitionURL', FALSE
          ),
          new StringAttribute (
            _('Acknowledgement URL'), _('Full URL to which functionnal acknowledgement should be sent'),
            'fdSinapsAcknowledgmentURL', FALSE
          ),
          new StringAttribute (
            _('Login'), _('Login to use for Basic Auth when contacting SINAPS services'),
            'fdSinapsLogin', FALSE,
            'fusiondirectory'
          ),
          new PasswordAttribute (
            _('Password'), _('Password to use for Basic Auth when contacting SINAPS services'),
            'fdSinapsPassword', FALSE
          ),
          new StringAttribute (
            _('Dump folder'), _('Folder in which received transactions should be dumped (leave empty to disable)'),
            'fdSinapsDumpFolder', FALSE
          ),
          new StringAttribute (
            _('Application identifier'), _('Application identifier present in cross references with FusionDirectory'),
            'fdSinapsIdentifiantApplication', FALSE
          ),
          new StringAttribute (
            _('UUID prefix'), _('Prefix used for UUID in supannRefId'),
            'fdSinapsUuidPrefix', TRUE,
            'LDAPUUID'
          ),
          new StringAttribute (
            _('User base'), _('Base in which users should be created when receiving a SINAPS diffusion'),
            'fdSinapsUserBase', TRUE,
            $config->current['BASE']
          ),
          new SelectAttribute (
            _('User template'), _('User template to use for user creation from SINAPS diffusion'),
            'fdSinapsUserTemplate', FALSE
          ),
          new SetAttribute (
            new StringAttribute (
              _('API Tokens'), _('One of these API tokens will need to be present in the diffusion URL used by SINAPS'),
              'fdSinapsFDToken', FALSE
            )
          ),
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE, $attributesInfo = NULL)
  {
    if ($attributesInfo === NULL) {
      $attributesInfo = $this->getAttributesInfo();
    }
    $userTemplates = objects::getTemplates('user');
    $attributesInfo['main']['attrs'][10]->setChoices(array_keys($userTemplates), array_values($userTemplates));

    parent::__construct($dn, $object, $parent, $mainTab, $attributesInfo);
  }
}
?>
