<?php
/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * and  compatible with GOsa 2.6 (http://www.gosa-project.org)
 * Copyright (C) 2011 Alejandro Escanero Blanco (aescanero@gmail.com)
 * Copyright (C) 2011-2013  FusionDirectory
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

class NisNetGroupSelectDialog
{
  private $select;
  private static $classSelect = 'memberNisnetgroupSelect';
  private $dialogAttribute;

  function __construct ($simplePlugin, $dialogAttribute)
  {
    $this->dialogAttribute  = $dialogAttribute;
    $this->select           = new self::$classSelect($simplePlugin->config, get_userinfo());
  }

  function execute ()
  {
    if (isset($_POST['add_memberNisnetgroups_cancel'])) {
      return FALSE;
    }
    if (isset($_POST['add_memberNisnetgroups_finish'])) {
      $groups = $this->select->detectPostActions();
      if (isset($groups['targets'])) {
        $headpage = $this->select->getHeadpage();
        foreach ($groups['targets'] as $dn) {
          $attrs = $headpage->getEntry($dn);

          $this->dialogAttribute->addValue($dn, $attrs);
        }
      }
      return FALSE;
    }
    session::set('filterBlacklist', $this->dialogAttribute->getFilterBlackList());
    return $this->select->execute();
  }
}

class NisNetGroupsAttribute extends GroupsAttribute
{
  protected $displays;
  protected $dialogClass = 'NisNetGroupSelectDialog';

  function addValue ($dn, $entry)
  {
    if (!in_array($dn, $this->value)) {
      $display = $entry['cn'][0];
      if (isset($entry['description'][0]) && !empty($entry['description'][0])) {
        $display .= ' ['.$entry['description'][0].']';
      }

      $this->value[]    = $dn;
      $this->displays[] = $display;
    }
  }
}

class netgroupAccount extends simplePlugin {
  var $objectclasses = array("whatever");

  static function plInfo()
  {
    return array(
      'plShortName'     => _('NIS Netgroup'),
      'plDescription'   => _('NIS Netgroup member'),
      'plSmallIcon'     => 'plugins/netgroups/images/netgroups.png',
      'plSelfModify'    => FALSE,
      'plPriority'      => 2,
      'plCategory'      => array('users'),
      'plObjectType'    => array('user'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      'groups' => array(
        'name'  => _('Member of the following NIS Netgroups  '),
        'icon'  => 'plugins/netgroups/images/netgroups.png',
        'attrs' => array(
          new NisNetGroupsAttribute('', _('NIS netgroup membership'), 'nisMembership')
        )
      ),
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $groupMembership = array();
    if (isset($object->uid)) {
      $uid = $object->uid;

      /* Groups handling */
      if ($this->dn != 'new') {
        $ldap = $this->config->get_ldap_link();
        $ldap->cd($this->config->current['BASE']);
        $ldap->search('(objectClass=nisNetgroup)', array('cn', 'nisNetgroupTriple'));
        while ($attrs = $ldap->fetch()) {
          if (isset($attrs['nisNetgroupTriple'])) {
            foreach ($attrs['nisNetgroupTriple'] as $val) {
              if (preg_match("/^\(\-?,".$uid.",\S*\)$/", $val)) {
                $desc = "";
                if (isset($attrs['description'])) {
                  $desc = $attrs['description'];
                }
                $groupMembership[$attrs['dn']] = $attrs['cn'][0]."[".$desc."]";
              }
            }
          }
        }
        asort($groupMembership);
        reset($groupMembership);
      }
    }
    $this->attributesAccess['nisMembership']->setInLdap(FALSE);
    $this->attributesAccess['nisMembership']->setValue(array_keys($groupMembership));
    $this->attributesAccess['nisMembership']->setDisplayValues(array_values($groupMembership));
    $this->attributesAccess['nisMembership']->setInitialValue(array_keys($groupMembership));
  }

  function is_this_account($attrs) {
    if (isset($attrs['uid'][0])) {
      $uid = $attrs['uid'][0];
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      $ldap->search('(objectClass=nisNetgroup)', array('cn', 'nisNetgroupTriple'));
      while ($attrs = $ldap->fetch()) {
        if (isset($attrs['nisNetgroupTriple'])) {
          foreach ($attrs['nisNetgroupTriple'] as $val) {
            if (preg_match("/^\(\-?,".$uid.",\S*\)$/", $val)) {
              return TRUE;
            }
          }
        }
      }
    }
    return FALSE;
  }

  /* Save data to LDAP, depending on is_account we save or delete */
  function ldap_save($cleanup = TRUE)
  {
    /* Call parents save to prepare $this->attrs */
    $ldap   = $this->config->get_ldap_link();
    $attrs  = array();
    $attrs['objectClass'] = array('top','nisNetgroup');

    $old_nisMembership  = $this->attributesAccess['nisMembership']->getInitialValue();
    $nisMembership      = $this->attributesAccess['nisMembership']->getValue();
    $to_add = array_diff($nisMembership, $old_nisMembership);
    $to_del = array_diff($old_nisMembership, $nisMembership);

    foreach ($to_add as $val) {
      $ldap->cd($this->config->current['BASE']);
      $ldap->cat($val, array("dn", "cn", "nisNetgroupTriple","memberNisNetgroups"));
      $attrs['nisNetgroupTriple'] = array();
      $attrs['memberNisNetgroup'] = array();
      while ($sattrs = $ldap->fetch()) {
        $triples    = array();
        $netgroups  = array();

        if (isset($sattrs['nisNetgroupTriple'])) {
          for ($i = 0;$i < $sattrs['nisNetgroupTriple']['count'];$i++) {
            $triples[] = $sattrs['nisNetgroupTriple'][$i];
          }
        }
        $triples[] = '(,'.$this->uid.',)';
        $attrs['nisNetgroupTriple'] = array_values(array_unique($triples));

        if (isset($sattrs['memberNisNetgroup'])) {
          for ($i = 0;$i < $sattrs['memberNisNetgroup']['count'];$i++) {
            $netgroups[] = $sattrs['memberNisNetgroup'][$i];
          }
        }
        $attrs['memberNisNetgroup'] = array_values(array_unique($netgroups));
        $ldap->cd($sattrs['dn']);
        $ldap->modify($attrs);
      }
    }

    foreach ($to_del as $val) {
      $ldap->cd($this->config->current['BASE']);
      $ldap->cat($val, array("dn", "cn", "nisNetgroupTriple","memberNisNetgroups"));
      $attrs['nisNetgroupTriple'] = array();
      $attrs['memberNisNetgroup'] = array();
      while ($sattrs = $ldap->fetch()) {
        $triples    = array();
        $netgroups  = array();

        if (isset($sattrs['nisNetgroupTriple'])) {
          for ($i = 0;$i < $sattrs['nisNetgroupTriple']['count'];$i++) {
            if (!preg_match("/^\(\-?," . $this->uid . ",\S*\)$/", $sattrs['nisNetgroupTriple'][$i])) {
              $triples[] = $sattrs['nisNetgroupTriple'][$i];
            }
          }
        }

        $attrs['nisNetgroupTriple'] = array_values(array_unique($triples));

        if (isset($sattrs['memberNisNetgroup'])) {
          for ($i = 0;$i < $sattrs['memberNisNetgroup']['count'];$i++) {
            $netgroups[] = $sattrs['memberNisNetgroup'][$i];
          }
        }
        $attrs['memberNisNetgroup'] = array_values(array_unique($netgroups));
        $ldap->cd($sattrs['dn']);
        $ldap->modify($attrs);
      }
    }
  }
}

?>
