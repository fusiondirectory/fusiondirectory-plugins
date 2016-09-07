<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
  Copyright (C) 2010 Antoine Gallavardin
  Copyright (C) 2011-2016 FusionDirectory

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

/* The UnitIntAttribute class handle the unit select box and convert the value to bytes.
 * The QuotaEntryAttribute takes care of converting that to a number of blocks using the blocksize server information
 *  it also handle soft and hard limits checking and device select update depending on server select */

class QuotaEntryAttribute extends CharSeparatedCompositeAttribute
{
  function readValues($value)
  {
    $values = parent::readValues($value);
    $blocksize = $this->plugin->getBlockSize($values[5], $values[0]);
    if ($blocksize !== FALSE) {
      $values[1] *= $blocksize;
      $values[2] *= $blocksize;
    }
    return $values;
  }

  function writeValues($values)
  {
    if (!empty($values[5])) {
      $blocksize = $this->plugin->getBlockSize($values[5], $values[0]);
      if ($blocksize !== FALSE) {
        $values[1] = ceil($values[1] / $blocksize);
        $values[2] = ceil($values[2] / $blocksize);
      }
    }
    return parent::writeValues($values);
  }

  function updateDeviceList()
  {
    $quotaDeviceList = $this->plugin->getDeviceList($this->attributes[5]->getValue());
    $this->attributes[0]->setChoices(array_keys($quotaDeviceList), array_values($quotaDeviceList));
  }

  function applyPostValue()
  {
    parent::applyPostValue();
    $this->updateDeviceList();
    /* Making sure hard limits are >= than soft limits */
    if ($this->attributes[2]->getValue() < $this->attributes[1]->getValue()) {
      $this->attributes[2]->setValue($this->attributes[1]->getValue());
    }
    if ($this->attributes[4]->getValue() < $this->attributes[3]->getValue()) {
      $this->attributes[4]->setValue($this->attributes[3]->getValue());
    }
  }

  function setValue ($values)
  {
    if (!is_array($values)) {
      $values = $this->inputValue($values);
    }
    $this->attributes[5]->setValue($values[5]);
    $this->updateDeviceList();
    for ($i = 0; $i < 5; $i++) {
      $this->attributes[$i]->setValue($values[$i]);
    }
  }

  function resetToDefault ()
  {
    $this->attributes[5]->resetToDefault();
    $this->updateDeviceList();
    for ($i = 0; $i < 5; $i++) {
      $this->attributes[$i]->resetToDefault();
    }
  }
}

class quotaAccount extends simplePlugin
{
  var $objectclasses = array('systemQuotas');

  var $displayHeader = TRUE;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Quota'),
      'plDescription' => _('Plugin for quota support'),
      'plIcon'        => 'geticon.php?context=applications&icon=quota&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=quota&size=16',
      'plSelfModify'  => TRUE,
      'plPriority'    => 8,
      'plObjectType'  => array('user'),
      'plDepends'     => array('posixAccount'),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'template'  => get_template_path('quota_section.tpl', TRUE, dirname(__FILE__)),
        'class'     => array('fullwidth'),
        'name'      => _('Quota informations'),
        'icon'      => 'geticon.php?context=applications&icon=quota&size=16',
        'attrs'     => array(
          new OrderedArrayAttribute(
            new QuotaEntryAttribute(
              _('Quota informations for this user'),
              'quota',
              array(
                new SelectAttribute(
                  _('Device'), _('Device this quota is for'),
                  'quotaDevice', TRUE
                ),
                new BytesSizeAttribute(
                  _('Block soft limit'), _('Soft limit for the block the user can use'),
                  'quotaBlockSoftLimit', TRUE,
                  // min
                  0,
                  // no max
                  FALSE,
                  0
                ),
                new BytesSizeAttribute(
                  _('Block hard limit'), _('Hard limit for the block the user can use'),
                  'quotaBlockHardLimit', TRUE,
                  // min
                  0,
                  // no max
                  FALSE,
                  0
                ),
                new IntAttribute(
                  _('Inode soft limit'), _('Soft limit for the inodes the user can use'),
                  'quotaInodeSoftLimit', TRUE,
                  // min
                  0,
                  // no max
                  FALSE,
                  0
                ),
                new IntAttribute(
                  _('Inode hard limit'), _('Hard limit for the inodes the user can use'),
                  'quotaInodeHardLimit', TRUE,
                  // min
                  0,
                  // no max
                  FALSE,
                  0
                ),
                new SelectAttribute(
                  _('Server'), _('Server hosting the device this quota is for'),
                  'quotaServer', TRUE
                ),
              ),
              ':'
            ),
            // no order
            FALSE,
            array(),
            // edit button
            TRUE
          )
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    /* Get List of server with Quota service */
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=quotaService)', array('cn','description','quotaDeviceParameters'));
    $this->quotaDeviceParameters = array();
    $quotaServersList = array();
    while ($attrs = $ldap->fetch()) {
      $this->quotaDeviceParameters[$attrs['cn'][0]] = $attrs['quotaDeviceParameters'];
      unset($this->quotaDeviceParameters[$attrs['cn'][0]]['count']);
      $display = $attrs['cn'][0];
      if (isset($attrs['description'][0]) && !empty($attrs['description'][0])) {
        $display .= ' - '.$attrs['description'][0];
      }
      $quotaServersList[$attrs['cn'][0]] = $display;
    }

    $attributesInfo = $this->getAttributesInfo();

    $attributesInfo['main']['attrs'][0]->setLinearRendering(FALSE);
    $attributesInfo['main']['attrs'][0]->attribute->attributes[5]->setSubmitForm(TRUE);
    $attributesInfo['main']['attrs'][0]->attribute->attributes[5]->setChoices(
      array_keys($quotaServersList), array_values($quotaServersList)
    );

    parent::__construct($dn, $object, $parent, $mainTab, $attributesInfo);
  }

  function getDeviceList($server)
  {
    $quotaDeviceList = array();
    if (isset($this->quotaDeviceParameters[$server])) {
      foreach ($this->quotaDeviceParameters[$server] as $infos) {
        $quotas = explode(":", $infos);
        $quotaDeviceList[$quotas[0]] = $quotas[0];
        if (!empty($quotas[2])) {
          $quotaDeviceList[$quotas[0]] .= ' - '.$quotas[2];
        }
      }
    }
    return $quotaDeviceList;
  }

  function getBlockSize($server, $device)
  {
    foreach ($this->quotaDeviceParameters[$server] as $infos) {
      $quotas = explode(":", $infos);
      if ($quotas[0] == $device) {
        return $quotas[1];
      }
    }
    return FALSE;
  }
}
?>
