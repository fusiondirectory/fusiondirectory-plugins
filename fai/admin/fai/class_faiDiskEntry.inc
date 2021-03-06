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

class PartitionDialog extends GenericDialog
{
  protected $dialogClass = 'faiPartition';

  protected $post_finish = 'PartitionSave';
  protected $post_cancel = 'PartitionCancel';

  protected $initialDialogValue = NULL;

  protected $plugin;

  function __construct ($simplePlugin, $attribute, $partition = [])
  {
    $this->attribute  = $attribute;
    $this->plugin     = $simplePlugin;
    $this->dialog = new $this->dialogClass($partition, $simplePlugin, $simplePlugin->FAIdiskType);

    $this->dialog->base = $simplePlugin->base;
    $this->dialog->set_acl_category('fai');
    if (!empty($partition)) {
      $this->initialDialogValue = $partition;
    }
  }

  protected function handleFinish ()
  {
    $this->dialog->update();

    $msgs = $this->dialog->check();
    if (count($msgs)) {
      msg_dialog::displayChecks($msgs);
      return;
    }

    $new_partition = $this->dialog->save();
    $msgs = $this->plugin->check_disks($new_partition);
    if (count($msgs)) {
      msg_dialog::displayChecks($msgs);
      return;
    }

    $this->attribute->updatePartition($new_partition);
    unset($this->dialog);
  }

  protected function handleCancel ()
  {
    if ($this->initialDialogValue !== NULL) {
      $this->attribute->addPartition($this->initialDialogValue);
    }
    unset($this->dialog);
  }
}

class DiskPartitionsAttribute extends DialogOrderedArrayAttribute
{
  protected $dialogClass = 'PartitionDialog';

  function __construct ($label, $description, $ldapName, $values = [], $acl = '')
  {
    parent::__construct($label, $description, $ldapName, FALSE, $values, $acl);
  }

  protected function loadAttrValue (array $attrs)
  {
  }

  /* Not saving anything into base node */
  function fillLdapValue (array &$attrs)
  {
  }

  protected function getAttributeArrayValue ($key, $value)
  {
    $opt = [];
    if (isset($value['bootable']) && $value['bootable']) {
      $opt[] = _('bootable');
    }
    if (!empty($value['preserve'])) {
      $opt[] = _('preserve').':'.$value['preserve'];
    }
    return [
      $value['description'],
      $value['FAIpartitionType'],
      $value['FAImountPoint'],
      $value['FAIpartitionSize'],
      implode(', ', $opt),
    ];
  }

  /* Only allow to remove partitions if they are not in use elsewhere */
  function delPostValue ($key)
  {
    /* Create a list of all partitions that are used in
     *  lvm, raid or cryptsetup compilations.
     */
    $list = [];
    foreach ($this->plugin->parent->disks as $disk) {
      if ($disk['FAIdiskType'] == 'lvm') {
        foreach ($disk['FAIlvmDevice'] as $partname) {
          $list[preg_replace('/:.*$/', '', $partname)][] = $disk;
        }
      } elseif ($disk['FAIdiskType'] == 'raid') {
        foreach ($disk['partitions'] as $part) {
          foreach (explode(',', $part['FAIpartitionSize']) as $partname) {
            $list[preg_replace('/:.*$/', '', $partname)][] = $disk;
          }
        }
      } elseif ($disk['FAIdiskType'] == 'cryptsetup') {
        foreach ($disk['partitions'] as $part) {
          $list[$part['FAIpartitionSize']][] = $disk;
        }
      }
    }

    /* Now that we've got a list of all partition references, lets check if
     *  one of the partitions we are going to remove is still in use.
     */
    if (isset($list[$this->postValue[$key]['cn']])) {
      $used = implode(',', array_values($list[$this->postValue[$key]['cn']]));
      $error = new FusionDirectoryError(
        htmlescape(sprintf(
          _('The partition cannot be deleted while it is used in the "%s" disk definition!'),
          $used
        ))
      );
      $error->display();
    } else {
      unset($this->postValue[$key]);
    }
  }

  /* Bypass delPostValue check for edition */
  protected function handleEdit ($key)
  {
    $this->editingValue = $this->value[$key];
    unset($this->postValue[$key]);
    $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this, $this->editingValue));
  }

  /* Add or update a partition */
  function updatePartition ($part)
  {
    if (!isset($part['FAIpartitionNr']) || ($part['FAIpartitionNr'] == 'undefined')) {
      $part['FAIpartitionNr'] = $this->get_free_partition_number();
    }

    if ($this->plugin->FAIdiskType == 'disk') {
      if (preg_match('/^disk(\d+)$/', $this->plugin->cn)) {
        $part['cn'] = $this->plugin->cn.'.'.$part['FAIpartitionNr'];
      } elseif (preg_match('/^nvme(\d+)/', $this->plugin->cn)) {
        $part['cn'] = $this->plugin->cn.'p'.$part['FAIpartitionNr'];
      } else {
        $part['cn'] = $this->plugin->cn.$part['FAIpartitionNr'];
      }
    } elseif ($this->plugin->FAIdiskType == 'lvm') {
      /* Check if we need to update partition names of lvm compilations */
      if (isset($this->value[$part['FAIpartitionNr']])) {
        $old_cn     = $this->value[$part['FAIpartitionNr']]['cn'];
        $new_cn     = $part['cn'];
        $lvmDevices = $this->plugin->lvmDevices;
        if (($key = array_search($old_cn, $lvmDevices)) !== FALSE) {
          $lvmDevices[$key]         = $new_cn;
          $this->plugin->lvmDevices = $lvmDevices;
        }
      }
    } elseif ($this->plugin->FAIdiskType == 'raid') {
      /* Set raid names to md# */
      $part['cn'] = 'md'.$part['FAIpartitionNr'];
    } elseif ($this->plugin->FAIdiskType == 'cryptsetup') {
      /* Set cryptsetup names to luks# */
      $part['cn'] = 'luks'.$part['FAIpartitionNr'];
    }

    $this->value[$part['FAIpartitionNr']] = $part;
  }

  /* Add a partition back */
  function addPartition ($part)
  {
    $this->value[$part['FAIpartitionNr']] = $part;
  }

  function get_free_partition_number ()
  {
    $used = [];
    foreach ($this->value as $key => $part) {
      $used[$key] = $part['FAIpartitionNr'];
    }
    $id = 1;
    while (in_array($id, $used) && $id < 16) {
      $id++;
    }
    return $id;
  }
}

class faiDiskEntry extends simplePlugin
{
  protected $is_edit = FALSE;

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Partition table entry'),
      'plDescription'   => _('FAI partition table entry'),
      'plCategory'      => ['fai'],
      /* No ACL, we use the one for FAIpartitions */
      'plProvidedAcls'  => []
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Device'),
        'attrs' => [
          new StringAttribute(
            _('Name'), _('Disk name'),
            'cn', TRUE,
            '', 'FAIpartitions',
            '/[a-z0-9_-]/i'
          ),
          new StringAttribute(
            _('Description'), _('Short description'),
            'description', FALSE
          ),
          new SelectAttribute(
            _('fstab key'), _('Key to use in fstab file'),
            'fstabkey', TRUE,
            ['device', 'label', 'uuid'], '',
            [_('Device'), _('Label'), _('UUID')]
          ),
          new SelectAttribute(
            _('Disk label'), _('Disk label'),
            'disklabel', TRUE,
            ['msdos', 'gpt'], '',
            ['MSDOS', 'GPT']
          ),
          new BooleanAttribute(
            _('Random init'), _('Initialise all encrypted partitions with random data'),
            'randinit', FALSE
          ),
          new HiddenAttribute('FAIdiskType')
        ]
      ],
      'partitions' => [
        'name'  => _('Partition entries'),
        'class' => ['fullwidth'],
        'attrs' => [
          new SetAttribute(
            new SelectAttribute(
              _('Combined physical partitions'), _('Physical partitions combined in this LVM volume'),
              'lvmDevices', FALSE,
              []
            )
          ),
          new DiskPartitionsAttribute(
            '', _('Partitions in this class'), 'partitions'
          )
        ]
      ]
    ];
  }

  function __construct ($dn, $parent, $disk, $type)
  {
    parent::__construct($dn, NULL, $parent);

    if ($disk) {
      $this->is_edit     = TRUE;

      $this->cn          = $disk['cn'];
      $this->description = $disk['description'];
      $this->attributesAccess['cn']->setInitialValue($this->cn);
    }

    $this->FAIdiskType  = $type;
    if ($this->FAIdiskType == 'lvm') {
      $this->attributesAccess['lvmDevices']->attribute->setChoices($this->getAvailablePartitions());
    } else {
      $this->attributesAccess['lvmDevices']->setDisabled(TRUE);
      $this->attributesAccess['lvmDevices']->setVisible(FALSE);
    }
    if ($this->FAIdiskType != 'disk') {
      $this->attributesAccess['disklabel']->setDisabled(TRUE);
      $this->attributesAccess['disklabel']->setVisible(FALSE);
      $this->attributesAccess['cn']->setDisabled(TRUE);
      $this->attributesAccess['cn']->setValue($this->FAIdiskType);
    }
    if ($this->FAIdiskType == 'cryptsetup') {
      $this->attributesAccess['fstabkey']->setDisabled(TRUE);
      $this->attributesAccess['fstabkey']->setVisible(FALSE);
    } else {
      $this->attributesAccess['randinit']->setDisabled(TRUE);
      $this->attributesAccess['randinit']->setVisible(FALSE);
    }

    if ($disk) {
      /* Get devices used in volume base disks */
      if (isset($disk['FAIlvmDevice'])) {
        $this->lvmDevices = $disk['FAIlvmDevice'];
      }

      if (isset($disk['FAIdiskOption'])) {
        foreach ($disk['FAIdiskOption'] as $option) {
          if (preg_match('/fstabkey:(device|label|uuid)/', $option, $m)) {
            $this->fstabkey = $m[1];
            continue;
          }

          if (preg_match('/disklabel:(msdos|gpt)/', $option, $m)) {
            $this->disklabel = $m[1];
            continue;
          }

          if (preg_match('/^bootable:(.*)$/', $option, $m)) {
            $bootable = preg_split('/,/', $m[1], -1, PREG_SPLIT_NO_EMPTY);
            foreach ($bootable as $bootflag) {
              if (isset($disk['partitions'][$bootflag])) {
                $disk['partitions'][$bootflag]['bootable'] = TRUE;
              }
            }
            continue;
          }

          if (preg_match('/^resize:(.*)$/', $option, $m)) {
            $resize = preg_split('/,/', $m[1], -1, PREG_SPLIT_NO_EMPTY);
            foreach ($resize as $id) {
              if (isset($disk['partitions'][$id])) {
                $disk['partitions'][$id]['resize'] = TRUE;
              }
            }
            continue;
          }

          // Load preserve_always flag for partitions
          if (preg_match('/^preserve_always:(.*)$/', $option, $m)) {
            $preserve = preg_split('/,/', $m[1], -1, PREG_SPLIT_NO_EMPTY);
            foreach ($preserve as $presflag) {
              if (isset($disk['partitions'][$presflag])) {
                $disk['partitions'][$presflag]['preserve'] = 'always';
              }
            }
            continue;
          }

          if (preg_match('/^preserve_reinstall:(.*)$/', $option, $m)) {
            $preserve = preg_split('/,/', $m[1], -1, PREG_SPLIT_NO_EMPTY);
            foreach ($preserve as $presflag) {
              if (isset($disk['partitions'][$presflag])) {
                $disk['partitions'][$presflag]['preserve'] = 'reinstall';
              }
            }
            continue;
          }

          if ($option == 'randinit') {
            $this->randinit = TRUE;
            continue;
          }
        }
      }

      $this->partitions = $disk['partitions'];
    }
  }

  protected function loadAttributes ()
  {
    foreach ($this->attributesAccess as &$attribute) {
      $attribute->setInLdap(FALSE);
      $attribute->setAcl('FAIpartitions');
    }
    unset($attribute);
    parent::loadAttributes();
  }

  public function render (): string
  {
    $smarty = get_smarty();
    $acl    = $this->aclGetPermissions('FAIpartitions');
    $smarty->assign('FAIpartitionsACL', $acl);
    parent::render();

    $buttons = '<p class="plugbottom">
                  <input type="submit" name="SaveDisk" value="'.msgPool::saveButton().'">
                  <input type="submit" formnovalidate="formnovalidate" name="CancelDisk" value="'.msgPool::cancelButton().'" >
                </p>';
    if ($this->displayPlugin) {
      return $this->header.$smarty->fetch($this->templatePath).$buttons;
    } else {
      return $this->header;
    }
  }

  function save (): array
  {
    $tmp = [];
    $tmp['cn']          = $this->cn;
    $tmp['description'] = $this->description;

    /* Attach partitions.
     * And prepare names and numbers.
     */
    $partitions = $this->partitions;
    foreach ($partitions as $key => $val) {
      $partitions[$key]['FAIpartitionNr'] = $key;
      if ($this->FAIdiskType == 'disk') {
        if (preg_match('/^disk(\d+)$/', $this->cn)) {
          $partitions[$key]['cn'] = $this->cn.'.'.$key;
        } else {
          $partitions[$key]['cn'] = $this->cn.$key;
        }
      } elseif ($this->FAIdiskType == 'lvm') {
        $partitions[$key]['FAIpartitionType'] = 'lvm';
      }
    }

    $tmp['partitions']  = $partitions;
    $tmp['FAIdiskType'] = $this->FAIdiskType;

    /* Add lvm devices if available */
    $tmp['FAIlvmDevice'] = $this->lvmDevices;

    /* Assemble flags */
    if ($this->FAIdiskType == 'disk') {
      $tmp['FAIdiskOption'] = ['fstabkey:'.$this->fstabkey, 'disklabel:'.$this->disklabel];
    } elseif ($this->FAIdiskType == 'cryptsetup') {
      if ($this->randinit) {
        $tmp['FAIdiskOption'] = ['randinit'];
      } else {
        $tmp['FAIdiskOption'] = [];
      }
    } else {
      $tmp['FAIdiskOption'] = ['fstabkey:'.$this->fstabkey];
    }

    /* If hdd name has changed, tell partitionTable to rename it */
    if ($this->is_edit && $this->attributesAccess['cn']->hasChanged()) {
      $tmp['rename']['from']  = $this->attributesAccess['cn']->getInitialValue();
      $tmp['rename']['to']    = $this->cn;
    }

    /* Build up disk options */
    $bootable           = '';
    $resize             = '';
    $preserve_always    = '';
    $preserve_reinstall = '';

    /* Assemble boot, resize and preserve flags */
    foreach ($tmp['partitions'] as $id => $part) {
      if (isset($part['bootable']) && $part['bootable']) {
        $bootable .= $id.',';
      }
      if (isset($part['resize']) && $part['resize']) {
        $resize .= $id.',';
      }
      if (isset($part['preserve'])) {
        if ($part['preserve'] == 'always') {
          $preserve_always .= $id.',';
        } elseif ($part['preserve'] == 'reinstall') {
          $preserve_reinstall .= $id.',';
        }
      }

      /* Unset non valid attributes */
      foreach (['bootable','preserve','resize','FAIdiskType'] as $attr) {
        if (isset($tmp['partitions'][$id][$attr])) {
          unset($tmp['partitions'][$id][$attr]);
        }
      }
    }

    /* Assemble disk flags */
    if (!empty($bootable)) {
      $tmp['FAIdiskOption'][] = 'bootable:'.trim($bootable, ',');
    }
    if (!empty($resize)) {
      $tmp['FAIdiskOption'][] = 'resize:'.trim($resize, ',');
    }
    if (!empty($preserve_always)) {
      $tmp['FAIdiskOption'][] = 'preserve_always:'.trim($preserve_always, ',');
    }
    if (!empty($preserve_reinstall)) {
      $tmp['FAIdiskOption'][] = 'preserve_reinstall:'.trim($preserve_reinstall, ',');
    }

    return $tmp;
  }

  /* Returns a list of available partitions that are useable in
   *  lvm disk setups.
   */
  function getAvailablePartitions (): array
  {
    $may  = [];
    $used = [];

    foreach ($this->parent->disks as $disk) {
      // Skip ourselves
      if ($disk['cn'] == $this->cn) {
        continue;
      }

      // Add partition from lvm combinations
      if ($disk['FAIdiskType'] == 'lvm') {
        $used = array_merge($used, $disk['FAIlvmDevice']);
      }

      foreach ($disk['partitions'] as $part) {
        // Add disks of raid arrays, to the used list.
        if ($disk['FAIdiskType'] == 'raid') {
          foreach (explode(',', $part['FAIpartitionSize']) as $rDevice) {
            $used[] = preg_replace('/:.*$/i', '', $rDevice);
          }
        } elseif ($disk['FAIdiskType'] == 'disk') {
          // Collect all available disks
          $name = $part['cn'];
          if (!in_array($name, $this->lvmDevices)) {
            $may[] = $name;
          }
        }
      }
    }

    // Check which of the available disks are unused.
    $ret = [];
    foreach ($may as $val) {
      if (!in_array($val, $used)) {
        $ret[$val] = $val;
      }
    }
    return $ret;
  }

  /* Checks the disk combinations */
  function check_disks ($disk_to_add = [])
  {
    $msgs = [];

    /* Check 'disk' combinations.
     *  - There can be four primary partitions.
     *  - If there is at least one 'logical' partition, then there can be only
     *     three 'primary' partitions.
     */
    if ($this->FAIdiskType == 'disk') {
      $types = ['logical' => [], 'primary' => []];
      $types[$disk_to_add['FAIpartitionType']][$disk_to_add['FAIpartitionNr']] = 1;
      foreach ($this->partitions as $part) {
        $types[$part['FAIpartitionType']][$part['FAIpartitionNr']] = 1;
      }

      // There can only be four primary partitions per disk - without any logical partition.
      if (count($types['logical']) == 0) {
        if (count($types['primary']) > 4) {
          $msgs[] = _('You have more than four primary partition table entries in your configuration');
        }
      } else {
        if (count($types['primary']) > 3) {
          $msgs[] = _('You cannot have more than three primary partition while using logical partitions');
        }
      }
    }
    return $msgs;
  }
}
