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

class FaiPartitionSizeAttribute extends CompositeAttribute
{
  function __construct ($label, $description, $ldapName, $acl = "")
  {
    parent::__construct (
      $description, $ldapName,
      array(
        new SelectAttribute(
          '', _('Type of sizing - fixed, dynamic or all remaining space'),
          'FAIpartitionSizeType', TRUE,
          array('fixed', 'dynamic', 'remaining'), '',
          array(_('Fixed'), _('Dynamic'), _('Remaining space'))
        ),
        new IntAttribute(
          '', '',
          'FAIpartitionSizeStart_size', FALSE,
          1, FALSE, ''
        ),
        new SelectAttribute(
          '', '',
          'FAIpartitionSizeStart_unit', TRUE,
          array('KB','MB','GB','TB','PB','%'), 'MB',
          array(_('KB'),_('MB'),_('GB'),_('TB'),_('PB'),'%')
        ),
        new IntAttribute(
          '-', '',
          'FAIpartitionSizeStop_size', FALSE,
          1, FALSE, ''
        ),
        new SelectAttribute(
          '', '',
          'FAIpartitionSizeStop_unit', TRUE,
          array('KB','MB','GB','TB','PB','%'), 'MB',
          array(_('KB'),_('MB'),_('GB'),_('TB'),_('PB'),'%')
        )
      ),
      '',
      '',
      $acl, $label
    );
    $this->setLinearRendering(TRUE);
    $this->attributes[0]->setSubmitForm(TRUE);
    $this->updateAttributes();
  }

  function readValues($value)
  {
    if (preg_match('/^([0-9]{1,})(KB|MB|GB|TB|PB|%)$/', $value, $m)) {
      // Fixed
      return array(
        'fixed',
        $m[1],$m[2],
        0,'',
      );
    } elseif (preg_match('/^([0-9]{1,})(KB|MB|GB|TB|PB|%)-([0-9]{1,})(KB|MB|GB|TB|PB|%)$/', $value)) {
      // Dynamic range
      return array(
        'dynamic',
        $m[1],$m[2],
        $m[3],$m[4],
      );
    } elseif (preg_match('/^0?\-$/', $value)) {
      // Dynamic range
      return array(
        'remaining',
        0,'',
        0,'',
      );
    }
  }

  protected function updateAttributes()
  {
    switch ($this->attributes[0]->getValue()) {
      case 'remaining':
        $this->attributes[1]->setVisible(FALSE);
        $this->attributes[2]->setVisible(FALSE);
        $this->attributes[3]->setVisible(FALSE);
        $this->attributes[4]->setVisible(FALSE);
      break;
      case 'fixed':
        $this->attributes[1]->setVisible(TRUE);
        $this->attributes[2]->setVisible(TRUE);
        $this->attributes[3]->setVisible(FALSE);
        $this->attributes[4]->setVisible(FALSE);
      break;
      case 'dynamic':
        $this->attributes[1]->setVisible(TRUE);
        $this->attributes[2]->setVisible(TRUE);
        $this->attributes[3]->setVisible(TRUE);
        $this->attributes[4]->setVisible(TRUE);
      break;
    }
  }

  function check ()
  {
    if ($this->isDisabled()) {
      return;
    }
    foreach ($this->attributes as &$attribute) {
      if (!$attribute->isVisible()) {
        continue;
      }
      $error = $attribute->check();
      if (!empty($error)) {
        return $error;
      }
    }
    unset($attribute);
    if ($this->attributes[0]->getValue() == 'dynamic') {
      $mp = array(
        '%'  => 1,
        'KB' => pow(1024,0),
        'MB' => pow(1024,1),
        'GB' => pow(1024,2),
        'TB' => pow(1024,3),
        'PB' => pow(1024,4)
      );
      $res1 = $this->attributes[1]->getValue() * $mp[$this->attributes[2]->getValue()];
      $res2 = $this->attributes[3]->getValue() * $mp[$this->attributes[4]->getValue()];
      if ($res1 > $res2) {
        return msgPool::toobig(_('Minimum partition size'), '\''._('Maximum partition size').'\'');
      }
    }
  }

  function applyPostValue()
  {
    parent::applyPostValue();
    $this->updateAttributes();
  }

  function setValue ($values)
  {
    parent::setValue($values);
    $this->updateAttributes();
  }

  function resetToDefault ()
  {
    parent::resetToDefault();
    $this->updateAttributes();
  }

  function writeValues($values)
  {
    if ($values[0] == 'remaining') {
      return '-';
    }
    $value = $values[1].$values[2];
    if ($values[0] == 'dynamic') {
      $value .= '-'.$values[3].$values[4];
    }
    return $value;
  }
}

class FaiMountPointAttribute extends StringAttribute
{
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = '/', $acl = '')
  {
    parent::__construct(
      $label, $description, $ldapName, $required,
      $defaultValue, $acl, '/^(\/|\-|swap)[a-z0-9%\/_.+-]*$/i', '/home'
    );
  }

  function setPostValue($value)
  {
    if ($value == '') {
      $value = '-';
    }
    parent::setPostValue($value);
  }
}

class FaiRaidDevices extends OrderedArrayAttribute
{
  protected function getAttributeArrayValue($key, $value)
  {
    $value = parent::getAttributeArrayValue($key, $value);
    return array(
      $value[0],
      ($value[1] ? 'spare' : ''),
      ($value[2] ? 'missing' : '')
    );
  }
}

class faiPartition extends simplePlugin
{
  static function plInfo()
  {
    return array(
      'plShortName'     => _('Partition entry'),
      'plDescription'   => _('FAI partition entry'),
      'plCategory'      => array('fai'),
      /* No ACL, we use the one for FAIpartitions */
      'plProvidedAcls'  => array()
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Partition'),
        'attrs' => array(
          new HiddenAttribute ('FAIdiskType'),
          new HiddenAttribute ('FAIpartitionNr', FALSE, 'undefined'),
          new SelectAttribute (
            _('Type'), _('Partition type'),
            'FAIpartitionType', TRUE,
            array('primary', 'logical'), '',
            array(_('Primary'), _('Logical'))
          ),
          new StringAttribute (
            _('Name'), _('Partition name'),
            'cn', TRUE,
            '', 'FAIpartitions',
            '/[a-z0-9_-]/i'
          ),
          new StringAttribute (
            _('Description'), _('Short description'),
            'description', FALSE
          ),
          new FaiPartitionSizeAttribute (
            _('Size'), _('Size of this partition'),
            'FAIpartitionSize', FALSE
          ),
        )
      ),
      'flags' => array(
        'name'  => _('Flags'),
        'attrs' => array(
          new CommaListAttribute(
            'raidDevices',
            new FaiRaidDevices(
              new CompositeAttribute(
                '', 'raidDevices_composite',
                array(
                  new SelectAttribute(
                    '', _('Combined physical partitions in this RAID'),
                    'raidDevices_select', FALSE
                  ),
                  new BooleanAttribute(
                    _('Spare'), _(''),
                    'raidDevices_spare', FALSE,
                    FALSE, '',
                    ':spare', ''
                  ),
                  new BooleanAttribute(
                    _('Missing'), _(''),
                    'raidDevices_missing', FALSE,
                    FALSE, '',
                    ':missing', ''
                  )
                ),
                '/^([^:]+)(:spare|)(:missing|)$/',
                '%s%s%s',
                '',
                ''
              ),
              FALSE,
              array(),
              TRUE
            )
          ),
          new BooleanAttribute (
            _('Resize'), _('Resize existing partition'),
            'resize', FALSE
          ),
          new BooleanAttribute (
            _('Bootable'), _('Wether or not this partition can be booted'),
            'bootable', FALSE
          ),
          new SelectAttribute (
            _('Preserve'), _(''),
            'preserve', FALSE,
            array('','always','reinstall'), '',
            array(_('Never'),_('Always'),_('Reinstall'))
          ),
        )
      ),
      'fs' => array(
        'name'  => _('Filesystem'),
        'attrs' => array(
          new SelectAttribute (
            _('Filesystem'), _('The filesystem this partition should be formatted with'),
            'FAIfsType', TRUE,
            array('swap','ext2','ext3','ext4','reiserfs','xfs','btrfs','-'), 'ext4',
            array(_('swap space'),_('ext2'),_('ext3'),_('ext4'),_('reiser fs'),_('xfs'),_('btrfs'),_('LVM/RAID'))
          ),
          new BooleanAttribute (
            _('Encrypted'), _('Wether or not this partition is encrypted'),
            'encrypted', FALSE
          ),
          new FaiMountPointAttribute (
            _('Mount point'), _('Mount point for this partition'),
            'FAImountPoint', TRUE
          ),
        )
      ),
      'options' => array(
        'name'  => _('Options'),
        'attrs' => array(
          new StringAttribute (
            _('Create options'), _('Filesystem create options'),
            'FAIfsCreateOptions', FALSE
          ),
          new StringAttribute (
            _('Tune options'), _('Filesystem tune options'),
            'FAIfsTuneOptions', FALSE
          ),
          new StringAttribute (
            _('Mount options'), _('Filesystem mount options'),
            'FAIfsOptions', FALSE
          ),
        )
      )
    );
  }

  function __construct($object, $parent, $type)
  {
    parent::__construct(NULL, NULL, $parent);

    $this->attributesAccess['FAIfsType']->setManagedAttributes(
      array('disable' => array ('swap' => array ('FAImountPoint')))
    );

    $this->FAIdiskType  = $type;

    if ($this->FAIdiskType == 'disk') {
      /* Check if we should be able to add primary partitions */
      $disablePrimary = FALSE;
      if (!$object || ($object['FAIpartitionType'] == 'logical')) {
        $types = array('logical' => array(), 'primary' => array());
        foreach ($this->parent->partitions as $key => $part){
          $types[$part['FAIpartitionType']][$part['FAIpartitionNr']] = 1;
        }
        if (count($types['logical']) && (count($types['primary']) >= 3)) {
          $disablePrimary = TRUE;
        } elseif (count($types['logical']) >= 4) {
          $disablePrimary = TRUE;
        }
      }
      if ($disablePrimary) {
        $types = array('logical' => _('Logical'));
      } else {
        $types  = array(
          'primary' => _('Primary'),
          'logical' => _('Logical')
        );
      }
    } elseif ($this->FAIdiskType == 'raid') {
      $types  = array(
        'raid0' => _('RAID 0'),
        'raid1' => _('RAID 1'),
        'raid5' => _('RAID 5'),
        'raid6' => _('RAID 6')
      );
    } else {
      $types = array();
    }
    $this->attributesAccess['FAIpartitionType']->setChoices(array_keys($types), array_values($types));
    if (empty($types)) {
      $this->attributesAccess['FAIpartitionType']->setVisible(FALSE);
      $this->attributesAccess['FAIpartitionType']->setDisabled(TRUE);
    }

    // Load attributes from existing partition
    if ($object) {
      if ($type == 'raid') {
        $object['raidDevices'] = $object['FAIpartitionSize'];
        unset($object['FAIpartitionSize']);
      }
      foreach ($this->attributes as $attr) {
        if (isset($object[$attr])) {
          $this->$attr = $object[$attr];
        }
      }

      if (($this->FAIdiskType == 'disk') || ($this->FAIdiskType == 'lvm')) {
        /* Check for encrypted partitions */
        if (preg_match('/:encrypt$/', $this->FAImountPoint)) {
          $this->FAImountPoint = preg_replace('/:encrypt/', '', $this->FAImountPoint);
          $this->encrypted = TRUE;
        }
      }
    }

    if ($this->FAIdiskType != 'lvm') {
      $this->attributesAccess['cn']->setDisabled(TRUE);
    }
    if ($this->FAIdiskType == 'raid') {
      $this->attributesAccess['raidDevices']->attributes[0]->attribute->attributes[0]->setChoices($this->getPartitionList());
      $this->attributesAccess['FAIpartitionSize']->setDisabled(TRUE);
      $this->attributesAccess['FAIpartitionSize']->setVisible(FALSE);
      $this->attributesInfo['flags']['name'] = _('Combined physical partitions');
      $this->attributesAccess['resize']->setDisabled(TRUE);
      $this->attributesAccess['resize']->setVisible(FALSE);
      $this->attributesAccess['preserve']->setDisabled(TRUE);
      $this->attributesAccess['preserve']->setVisible(FALSE);
      $this->attributesAccess['bootable']->setDisabled(TRUE);
      $this->attributesAccess['bootable']->setVisible(FALSE);
    } else {
      $this->attributesAccess['raidDevices']->setDisabled(TRUE);
      $this->attributesAccess['raidDevices']->setVisible(FALSE);
    }
  }

  protected function loadAttributes()
  {
    foreach ($this->attributesAccess as &$attribute) {
      //~ $attribute->setInLdap(FALSE);
      $attribute->setAcl('FAIpartitions');
    }
    unset($attribute);
    parent::loadAttributes();
  }

  function execute()
  {
    $smarty = get_smarty();
    $acl    = $this->getacl('FAIpartitions');
    $smarty->assign('FAIpartitionsACL', $acl);
    parent::execute();

    $buttons = '<p class="plugbottom">
                  <input type="submit" name="PartitionSave" value="'.msgPool::saveButton().'">
                  <input type="submit" name="PartitionCancel" value="'.msgPool::cancelButton().'" >
                </p>';
    if ($this->displayPlugin) {
      return $this->header.$smarty->fetch($this->templatePath).$buttons;
    } else {
      return $this->header;
    }
  }

  function save_object()
  {
    parent::save_object();
    if ($this->FAIfsType == 'swap') {
      $this->FAImountPoint = 'swap';
    }
  }

  function check()
  {
    $messages = parent::check();

    if ($this->FAIdiskType == 'raid') {
      $raidDevices = $this->attributesAccess['raidDevices']->attributes[0]->getValue();
      if (count($raidDevices) < 2) {
        $messages[] = _('Raid arrays must contain at least two partitions!');
      } elseif (($this->FAIpartitionType == 'raid0') && (count($this->raidDevices) != 2)) {
        $messages[] = _('Raid 0 arrays can only be realized with a combination of two partitions!');
      }
    }

    return $messages;
  }

  function save()
  {
    $ret = array();
    foreach ($this->attributes as $attr) {
      $ret[$attr] = $this->$attr;
    }

    if (($this->FAIdiskType == 'disk') || ($this->FAIdiskType == 'lvm')) {
      /* Add encryption flag to partition mount point */
      if ($this->encrypted) {
        $ret['FAImountPoint'] .= ':encrypt';
      }
    } elseif($this->FAIdiskType == 'raid') {
      $ret['FAIpartitionSize'] = $ret['raidDevices'];
      unset($ret['raidDevices']);
    }

    return $ret;
  }

  /* Returns a list of all partitions that are useable
   *  for raid arrays.
   */
  function getPartitionList()
  {
    $may = $used = array();
    foreach ($this->parent->parent->disks as $disk) {
      // Skip ourselves
      if ($disk['cn'] == $this->parent->cn) {
        continue;
      }

      // Add partition from lvm combinations
      if ($disk['FAIdiskType'] == 'lvm') {
        $used = array_merge($used,$disk['FAIlvmDevice']);
      }

      foreach ($disk['partitions'] as $key => $part) {
        // Add disks of raid arrays, to the used list.
        if ($disk['FAIdiskType'] == 'raid') {
          foreach (explode(',', $part['FAIpartitionSize']) as $rDevice) {
            $used[] = preg_replace('/:.*$/i', '', $rDevice);
          }
        }

        // Collect all available disks
        if ($disk['FAIdiskType'] == 'disk') {
          $name = $part['cn'];
          if (!isset($this->raidDevices[$name])) {
            $may[] = $name;
          }
        }
      }
    }

    // Check which of the available disks are unused.
    $ret = array();
    foreach ($may as $val) {
      if (!in_array($val, $used)) {
        $ret[$val] = $val;
      }
    }
    return $ret;
  }
}
?>
