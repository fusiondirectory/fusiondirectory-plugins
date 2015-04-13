<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2015  FusionDirectory

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

class PartitionDialog extends GenericDialog
{
  protected $dialogClass = 'faiDiskEntry';

  protected $post_cancel = 'CancelDisk';
  protected $post_finish = 'SaveDisk';

  protected $initialDialogValue = NULL;

  function __construct($simplePlugin, &$attribute, $disk = array(), $diskType = 'disk')
  {
    $this->attribute = $attribute;
    if (isset($disk['FAIdiskType'])) {
      $diskType = $disk['FAIdiskType'];
    }
    $this->dialog = new $this->dialogClass($simplePlugin->config, $simplePlugin->dn, $attribute, $disk, $diskType);

    $base = $simplePlugin->dn;
    if ($base == 'new') {
      $base = $simplePlugin->base;
    }
    $this->dialog->set_acl_base($base);
    $this->dialog->set_acl_category('fai');
    if (!empty($disk)) {
      $this->initialDialogValue = $disk;
    }
  }

  function dialog_execute ()
  {
    $this->dialog->save_object();
    return $this->dialog->execute();
  }

  function handle_finish ()
  {
    $this->dialog->save_object();
    if (count($this->dialog->check())) {
      foreach ($this->dialog->check() as $msg) {
        msg_dialog::display(_('Error'), $msg, ERROR_DIALOG);
      }
      return $this->dialog->execute();
    }

    $this->attribute->diskConfigured($this->dialog->save());
    return FALSE;
  }

  function handle_cancel ()
  {
    if ($this->initialDialogValue !== NULL) {
      $this->attribute->addValue($this->initialDialogValue);
    }
    return FALSE;
  }
}

class PartitionTableAttribute extends DialogOrderedArrayAttribute
{
  protected $dialogClass = 'PartitionDialog';
  protected $partitionAttributes  = array(
    'cn','description',
    'FAIpartitionNr',   'FAIpartitionSize',   'FAImountPoint',
    'FAIfsType',        'FAIpartitionType',   'FAImountOptions',
    'FAIfsOptions',     'FAIpartitionFlags',  'FAIfsCreateOptions',
    'FAIfsTuneOptions', 'FAIfsOptions',       'FAIpartitionFlags',
    'FAIlvmDevice'
  );

  protected $diskFilter = '(&(objectClass=FAIclass)(objectClass=FAIpartitionDisk))';
  protected $partFilter = '(&(objectClass=FAIclass)(objectClass=FAIpartitionEntry))';

  public $disks;

  function __construct ($label, $description, $ldapName, $values = array(), $acl = '')
  {
    parent::__construct ($label, $description, $ldapName, FALSE, $values, $acl);
    $this->disks = &$this->value;
  }

  function renderButtons ()
  {
    $id = $this->getHtmlId();
    return $this->renderInputField(
      'submit',
      'add'.$id.'_dialog',
      array('value' => '{msgPool type=addButton}')
    ).$this->renderInputField(
      'submit',
      'add'.$id.'_dialog_raid',
      array('value' => _('Add RAID'))
    ).$this->renderInputField(
      'submit',
      'add'.$id.'_dialog_lvm',
      array('value' => _('Add LVM'))
    );
  }

  function loadPostValue ()
  {
    $id = $this->getHtmlId();
    if ($this->isVisible()) {
      parent::loadPostValue();
      parent::applyPostValue();
      if (isset($_POST['add'.$id.'_dialog'])) {
        $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this, array(), 'disk'));
      } elseif (isset($_POST['add'.$id.'_dialog_raid'])) {
        $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this, array(), 'raid'));
      } elseif (isset($_POST['add'.$id.'_dialog_lvm'])) {
        $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this, array(), 'lvm'));
      }
    }
  }

  protected function loadAttrValue ($attrs)
  {
    if (isset($attrs['dn'])) {
      /* Fetch disks */
      $ldap = $this->plugin->config->get_ldap_link();
      $ldap->ls($this->diskFilter, $attrs['dn']);
      $this->value = array();
      while ($subattrs = $ldap->fetch()) {
        $diskInfos = array('description' => '', 'FAIdiskType' => 'disk');
        $diskInfos['description']  = '';
        foreach (array('cn', 'description','FAIdiskType') as $attr) {
          if (isset($subattrs[$attr][0])) {
            $diskInfos[$attr] = $subattrs[$attr][0];
          }
        }

        // Get disk options, without 'count' index.
        $diskInfos['FAIdiskOption'] = array();
        if (isset($subattrs['FAIdiskOption'])) {
          for ($i = 0; $i < $subattrs['FAIdiskOption']['count']; $i++) {
            $diskInfos['FAIdiskOption'][] = $subattrs['FAIdiskOption'][$i];
          }
        }

        // Transform potential lvm information
        if (isset($subattrs['FAIlvmDevice'])) {
          for ($i = 0; $i < $subattrs['FAIlvmDevice']['count']; $i++) {
            $name = $subattrs['FAIlvmDevice'][$i];
            $diskInfos['FAIlvmDevice'][$name] = $name;
          }
        }

        $diskInfos['dn'] = $subattrs['dn'];
        $this->value[$diskInfos['dn']] = $diskInfos;
        $this->value[$diskInfos['dn']]['partitions'] = array();
      }
      /* Fetch partitions */
      foreach ($this->value as $name => $disk) {
        $ldap->ls($this->partFilter, $disk['dn']);
        while ($obj = $ldap->fetch()) {
          $partitionInfos = array();
          foreach ($this->partitionAttributes as $attr) {
            if (isset($obj[$attr][0])) {
              $partitionInfos[$attr] = $obj[$attr][0];
            }
          }
          $this->value[$name]['partitions'][$partitionInfos['FAIpartitionNr'][0]] = $partitionInfos;
        }
      }
    } else {
      $this->resetToDefault();
    }
  }

  /* Not saving anything into base node */
  function fillLdapValue (&$attrs)
  {
    /* Remove crap made by plugin */
    unset ($attrs[$this->getLdapName()]);
  }

  /* Special LDAP treatment that this attribute does after plugin ldap save */
  function postLdapSave ($ldap)
  {
    /* First delete all old nodes */
    $ldap->ls($this->diskFilter, $this->plugin->dn, array('dn'));
    $delete = array();
    while ($attrs = $ldap->fetch()) {
      $delete[] = $attrs['dn'];
    }
    foreach ($delete as $dn) {
      $ldap->rmdir_recursive($dn);
      if (!$ldap->success()) {
        msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $dn, LDAP_ADD, get_class()), LDAP_ERROR);
      }
    }
    /* Then add our values */
    foreach ($this->value as $val) {
      $attrs = $val;
      $attrs['objectClass'] = array('top','FAIClass','FAIpartitionDisk');
      unset($attrs['partitions']);
      $disk_dn = $this->compute_disk_dn($val);
      $this->ldap_add($ldap, $disk_dn, $attrs);
      /* disk added, now add partition */
      foreach ($val['partitions'] as $part) {
        $attrs = $part;
        $attrs['objectClass'] = array('top','FAIClass','FAIpartitionEntry');
        $dn = $this->compute_part_dn($part, $disk_dn);
        $this->ldap_add($ldap, $dn, $attrs);
      }
    }
  }

  function compute_disk_dn ($disk)
  {
    return 'cn='.$disk['cn'].','.$this->plugin->dn;
  }

  function compute_part_dn ($part, $disk_dn)
  {
    return 'FAIpartitionNr='.$part['FAIpartitionNr'].','.$disk_dn;
  }

  protected function ldap_add ($ldap, $dn, $attrs)
  {
    unset($attrs['dn']);
    $ldap->cd($dn);
    foreach (array_keys($attrs) as $index) {
      if ((is_array($attrs[$index]) && (count($attrs[$index]) == 0)) || ($attrs[$index] == '')) {
        unset($attrs[$index]);
      }
    }
    $ldap->add($attrs);
    if (!$ldap->success()) {
      msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $dn, LDAP_ADD, get_class()), LDAP_ERROR);
    }
  }

  protected function getAttributeArrayValue($value)
  {
    return array(
      $value['FAIdiskType'],
      $value['cn'],
      $value['description'],
      count($value['partitions']),
    );
  }

  function diskConfigured ($disk)
  {
    if (isset($disk['rename'])) {
      unset($this->value[$disk['rename']['from']]);
      $disk['cn'] = $disk['rename']['to'];
    }

    $this->value[$disk['cn']] = $disk;
  }
}

class faiPartitionTable extends faiSimplePluginClass
{
  var $mainTab = TRUE;

  var $objectclasses    = array('top','FAIclass','FAIpartitionTable');

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Partition table'),
      'plDescription' => _('FAI partition table'),
      'plObjectType'  => array(
        'faiPartitionTable' => array(
          'name'        => _('FAI partition table'),
          'filter'      => 'objectClass=FAIpartitionTable',
          'aclCategory' => 'fai',
          'ou'          => get_ou('faiPartitionTableRDN'),
          'icon'        => 'geticon.php?context=applications&icon=fai-partitiontable&size=16'
        )
      ),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Properties'),
        'attrs' => array(
          new HostNameAttribute (
            _('Class name'), _('Partition table class name'),
            'cn', TRUE
          ),
          new StringAttribute (
            _('Description'), _('Short description of the class'),
            'description', FALSE
          ),
        )
      ),
      'table' => array(
        'name'  => _('Discs'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new HiddenAttribute (
            'FAIpartitionMethod', FALSE, 'setup-storage'
          ),
          new PartitionTableAttribute (
            '', _('Partitions in this class'), 'FAIpartitions'
          )
        )
      )
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->attributesAccess['cn']->setUnique(TRUE);
  }

  function ldap_save ($cleanup = TRUE)
  {
    parent::ldap_save($cleanup);
    $this->attributesAccess['FAIpartitions']->postLdapSave($this->config->get_ldap_link());
  }

  function getUsedDiskNames()
  {
    $ret = array();
    foreach ($this->FAIpartitions as $disk) {
      $ret[] = $disk['cn'];
    }
    return $ret;
  }
}
?>
