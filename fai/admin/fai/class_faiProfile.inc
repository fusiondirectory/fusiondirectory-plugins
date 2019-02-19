<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2014-2016 FusionDirectory

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

class faiProfile extends faiSimplePluginClass
{
  static function plInfo (): array
  {
    return array(
      'plShortName'   => _('Profile'),
      'plDescription' => _('FAI profile'),
      'plObjectClass' => array('FAIclass','FAIprofile'),
      'plFilter'      => '(objectClass=FAIprofile)',
      'plObjectType'  => array(
        'faiProfile' => array(
          'name'        => _('FAI Profile'),
          'aclCategory' => 'fai',
          'ou'          => get_ou('faiProfileRDN'),
          'icon'        => 'geticon.php?context=applications&icon=fai-profile&size=16'
        )
      ),
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo (): array
  {
    return array(
      'main' => array(
        'name'  => _('Properties'),
        'attrs' => array(
          new HostNameAttribute(
            _('Class name'), _('Partition table class name'),
            'cn', TRUE
          ),
          new StringAttribute(
            _('Description'), _('Short description of the class'),
            'description', FALSE
          ),
          new CommaListAttribute(
            'FAIclass',
            new SetAttribute(
              new SelectAttribute(
                _('FAI classes'), _('FAI classes which are part of this profile'),
                'FAIclass_set', TRUE
              )
            ),
            ' '
          )
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['cn']->setUnique('one');

    /* Build filter */
    $types    = array('faiVariable', 'faiHook', 'faiScript', 'faiPackage', 'faiPartitionTable', 'faiTemplate');
    $choices  = array();
    foreach ($types as $type) {
      $choices = array_unique(array_merge($choices, objects::ls($type, NULL, $this->base)));
    }
    sort($choices);

    $this->attributesAccess['FAIclass']->attributes[0]->attribute->setChoices($choices);
  }
}
?>
