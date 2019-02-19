<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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

class FAIreleaseAttribute extends SelectAttribute
{
  function __construct ($label, $description, $ldapName, $required)
  {
    parent::__construct($label, $description, $ldapName, $required);
    $this->setSubmitForm(TRUE);
  }

  function setValue ($value)
  {
    if (!in_array($value, $this->choices)) {
      return;
    }
    parent::setValue($value);
    if (isset($this->plugin)) {
      $this->plugin->attributesAccess['FAIclass']->attributes['profil']->setChoices($this->plugin->cache['PROFILES'][$value]);
      $this->plugin->attributesAccess['FAIdebianMirror']->setChoices($this->plugin->cache['SERVERS'][$value]);
    }
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    $this->setChoices(array_keys($this->plugin->cache['SERVERS']));
  }
}

class faiStartup extends simplePlugin
{
  var $displayHeader = TRUE;

  var $inheritance    = array('gosaGroupOfNames' => 'member');
  var $sdaemon_available;

  var $cache = array();

  static function plInfo (): array
  {
    return array(
      'plShortName'     => _('FAI'),
      'plDescription'   => _('Full automated installation'),
      'plPriority'      => 9,
      'plObjectClass'   => array('FAIobject'),
      'plObjectType'    => array('workstation', 'server', 'ogroup-dynamic'),
      'plForeignKeys'   => array(
        'FAIclass'   => array(
          array('faiProfile', 'cn', '(FAIclass=%oldvalue% :*)'),
        ),
      ),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo (): array
  {
    return array(
      'main' => array(
        'name'  => _('FAI settings'),
        'attrs' => array(
          new HiddenAttribute('FAIstate'),
          new CompositeAttribute(
            _('FAI profil and release to be applied to this computer'),
            'FAIclass',
            array(
              'release' => new FAIreleaseAttribute(
                _('Release'), _('FAI debian release to be installed on this computer'),
                'FAIrelease', TRUE
              ),
              'profil' => new SelectAttribute(
                _('Profil'), _('FAI profil to be applied to this computer'),
                'FAIprofile', TRUE
              ),
            ),
            '/^(?P<profil>.+) :(?P<release>.+)$/',
            '%2$s :%1$s'
          ),
          new SelectAttribute(
            _('Repository'), _('FAI Debian repository to be used for installation'),
            'FAIdebianMirror', TRUE
          ),
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    $s_daemon = new supportDaemon();
    $this->sdaemon_available = $s_daemon->is_available();

    $this->update_fai_cache();

    parent::__construct($dn, $object, $parent, $mainTab);
  }

  function foreignKeyCheck ($field, $fieldvalue, $source)
  {
    if ($field == 'FAIclass') {
      if ($this->attributesAccess['FAIclass']->attributes['profil']->getValue() == $fieldvalue) {
        return TRUE;
      }
      return FALSE;
    } else {
      return parent::foreignKeyCheck($field, $fieldvalue, $source);
    }
  }

  function foreignKeyUpdate (string $field, $oldvalue, $newvalue, array $source)
  {
    if ($field == 'FAIclass') {
      if ($this->attributesAccess['FAIclass']->attributes['profil']->getValue() == $oldvalue) {
        $this->attributesAccess['FAIclass']->attributes['profil']->setValue($newvalue);
      }
    } else {
      return parent::foreignKeyUpdate($field, $oldvalue, $newvalue, $source);
    }
  }

  /* Updates release and profiles list
   *  if not already done ($this->cache).
   */
  function update_fai_cache ()
  {
    /* Get the list of available servers and their releases */
    if (!isset($this->cache['SERVERS'])) {
      $tmp = serviceRepository::getServers();
      $this->cache['SERVERS'] = array();
      foreach ($tmp as $entry) {
        if ($entry['INSTALL'] != 'install') {
            continue;
        }
        $rel = $entry['FAI_RELEASE'];
        if (!isset($this->cache['SERVERS'][$rel])) {
          $this->cache['SERVERS'][$rel] = array();
        }
        $this->cache['SERVERS'][$rel][] = $entry['URL'];
        uksort($this->cache['SERVERS'][$rel], 'strnatcasecmp');
      }
    }

    if (!isset($this->cache['PROFILES'])) {
      $this->cache['PROFILES'] = array();
      foreach (array_keys($this->cache['SERVERS']) as $release) {
        /* Get the list of available profiles for each release */
        $tmp = objects::ls('faiProfile', 'cn', NULL, "(ou:dn:=$release)");

        if (empty($tmp)) {
          unset($this->cache['SERVERS'][$release]);
        } else {
          $this->cache['PROFILES'][$release] = $tmp;
        }
      }
    }
  }
}

?>
