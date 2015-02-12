<?php
/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * and  compatible with GOsa 2.6 (http://www.gosa-project.org)
 * Copyright (C) 2011 Alejandro Escanero Blanco (aescanero@gmail.com)
 * Copyright (C) 2011-2015  FusionDirectory
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

class NetgroupSelectDialog extends GenericSelectDialog
{
  protected $dialogClass = 'memberNisnetgroupSelect';
}

class NetgroupsAttribute extends GroupsAttribute
{
  protected $dialogClass = 'NetgroupSelectDialog';
}

class netgroupMembership extends simplePlugin {
  var $displayHeader = TRUE;
  var $objectclasses = array("whatever");
  var $id;
  var $old_id;
  protected static $id_attr = 'uid';

  static function plInfo()
  {
    return array(
      'plShortName'     => _('NIS Netgroup'),
      'plDescription'   => _('NIS Netgroup member'),
      'plIcon'          => 'plugins/netgroups/images/icon.png',
      'plSmallIcon'     => 'plugins/netgroups/images/iconMini.png',
      'plSelfModify'    => FALSE,
      'plPriority'      => 13,
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
        'icon'  => 'plugins/netgroups/images/iconMini.png',
        'attrs' => array(
          new NetgroupsAttribute('', _('NIS netgroup membership'), 'nisMembership', TRUE)
        )
      ),
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $groupMembership = array();

    /* Groups handling */
    if ($this->dn != 'new') {
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      $ldap->search('(objectClass=nisNetgroup)', array('cn', 'nisNetgroupTriple'));
      while ($attrs = $ldap->fetch()) {
        if (isset($attrs['nisNetgroupTriple'])) {
          foreach ($attrs['nisNetgroupTriple'] as $val) {
            if (preg_match($this->triple_mask(), $val)) {
              $label = $attrs['cn'][0];
              if (isset($attrs['description'])) {
                $label .= ' ['.$attrs['description'][0].']';
              }
              $groupMembership[$attrs['dn']] = $label;
            }
          }
        }
      }
      asort($groupMembership);
      reset($groupMembership);
    }
    $this->attributesAccess['nisMembership']->setInLdap(FALSE);
    $this->attributesAccess['nisMembership']->setValue(array_keys($groupMembership));
    $this->attributesAccess['nisMembership']->setDisplayValues(array_values($groupMembership));
    $this->attributesAccess['nisMembership']->setInitialValue(array_keys($groupMembership));
  }

  protected function triple_mask()
  {
    return "/^\(\-?,".$this->old_id.",\S*\)$/";
  }

  protected function triple_data()
  {
    return '(,'.$this->id.',)';
  }

  protected function update_id()
  {
    $class    = get_class($this);
    $id_attr  = $class::$id_attr;
    if (isset($this->parent)) {
      $baseobject = $this->parent->getBaseObject();
      if (isset($baseobject->$id_attr)) {
        $this->id = $baseobject->$id_attr;
        return;
      }
    }
    if (isset($this->attrs[$id_attr])) {
      $this->id = $this->attrs[$id_attr][0];
    }
  }

  function is_this_account($attrs)
  {
    $class    = get_class($this);
    $id_attr  = $class::$id_attr;
    if (isset($attrs[$id_attr])) {
      $this->id     = $attrs[$id_attr][0];
      $this->old_id = $this->id;
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      $ldap->search('(objectClass=nisNetgroup)', array('cn', 'nisNetgroupTriple'));
      while ($attrs = $ldap->fetch()) {
        if (isset($attrs['nisNetgroupTriple'])) {
          foreach ($attrs['nisNetgroupTriple'] as $val) {
            if (preg_match($this->triple_mask(), $val)) {
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
    $this->update_id();
    /* Call parents save to prepare $this->attrs */
    $ldap   = $this->config->get_ldap_link();
    $attrs  = array();
    $attrs['objectClass'] = array('top','nisNetgroup');

    $old_nisMembership  = $this->attributesAccess['nisMembership']->getInitialValue();
    $nisMembership      = $this->attributesAccess['nisMembership']->getValue();
    if ($this->old_id == $this->id) {
      $to_add = array_diff($nisMembership, $old_nisMembership);
      $to_del = array_diff($old_nisMembership, $nisMembership);
    } else {
      $to_add = $nisMembership;
      $to_del = $old_nisMembership;
    }

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
        $triples[] = $this->triple_data();
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
            if (!preg_match($this->triple_mask(), $sattrs['nisNetgroupTriple'][$i])) {
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
