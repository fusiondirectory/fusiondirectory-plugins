<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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
      $this->plugin->attributesInfo['main']['attrs']['FAIclass']->attributes[1]->setChoices($this->plugin->cache['PROFILES'][$value]);
      $this->plugin->attributesInfo['main']['attrs']['FAIdebianMirror']->setChoices($this->plugin->cache['SERVERS'][$value]);
      $this->plugin->attributesInfo['startup']['attrs']['gotoBootKernel']->setRelease($value);
    }
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    $this->setChoices(array_keys($this->plugin->cache['SERVERS']));
  }
}

class BootKernelAttribute extends SelectAttribute
{
  private $cache = array();

  function setRelease($release)
  {
    if ($this->update_kernel_cache($release)) {
      $this->setChoices($this->cache[$release]);
      $this->setDisabled(FALSE);
    } else {
      $this->setChoices(array());
      $this->setValue($this->getInitialValue());
      $this->setDisabled(TRUE);
    }
  }

  function update_kernel_cache($release)
  {
    /* Get list of available kernel for this release
     */
    if (!isset($this->cache)) {
      $this->cache = array();
    }

    if (!isset($this->cache[$release])) {
      $s_daemon = new supportDaemon();
      if (!$s_daemon->is_available()) {
        return FALSE;
      }
      $tmp = $s_daemon->FAI_get_kernels($release);
      if ($s_daemon->is_error()) {
        return FALSE;
      }
      $this->cache[$release] = $tmp;
      ksort($this->cache[$release]);
    }
    return TRUE;
  }
}

class faiStartup extends simplePlugin
{
  var $displayHeader = TRUE;

  /* attribute list for save action */
  var $objectclasses        = array("FAIobject");

  var $inheritance = array("gotoWorkstationTemplate" => "member");
  var $sdaemon_available;

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('FAI settings'),
        'attrs' => array(
          new CompositeAttribute (
            _('FAI profil and release to be applied to this computer'),
            'FAIclass',
            array(
              new FAIreleaseAttribute (
                _('Release'), _('FAI debian release to be installed on this computer'),
                'FAIrelease', TRUE
              ),
              new SelectAttribute (
                _('Profil'), _('FAI profil to be applied to this computer'),
                'FAIprofile', TRUE
              ),
            ),
            '%2$[^ :] :%1$[^ :]',
            '%2$s :%1$s'
          ),
          new SelectAttribute (
            _('Repository'), _('FAI Debian repository to be used for installation'),
            'FAIdebianMirror', TRUE
          ),
        )
      ),
      'startup' => array (
        'name' => _('Startup parameters'),
        'attrs' => array (
          new BootKernelAttribute (
            _('Boot kernel'),  _('Linux kernel to be installed and booted'),
            'gotoBootKernel', FALSE
          ),
          new StringAttribute (
            _('Boot kernel parameters'), _('Linux kernel parameters to be ran'),
            'gotoKernelParameters', FALSE
          )
        )
      ),
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    $s_daemon = new supportDaemon();
    $this->sdaemon_available = $s_daemon->is_available();

    $this->update_fai_cache();

    parent::__construct($config, $dn, $object);
  }

  /* Updates release dns
   *  and reads all classes for the current release,
   *  if not already done ($this->cache).
   */
  function update_fai_cache($release = FALSE)
  {
    /* Get the list of available servers and their releases.
     */
    if (!isset($this->cache['SERVERS'])) {
      $o_queue = new supportDaemon();
      $tmp = $o_queue->FAI_get_server();
      if ($o_queue->is_error()) {
        msg_dialog::display(_("Service infrastructure"), msgPool::siError($o_queue->get_error()), ERROR_DIALOG);
        $this->cache = array();
        return;
      } else {
        $this->cache['SERVERS'] = array();
        foreach ($tmp as $entry) {
          if ($entry['INSTALL'] != "install") {
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
    }

    if (!isset($this->cache['PROFILES'])) {
      $this->cache['PROFILES'] = array();
      $o_queue = new supportDaemon();
      foreach (array_keys($this->cache['SERVERS']) as $release) {
        /* Get the list of available servers and their releases.
         */
        $tmp = $o_queue->FAI_get_profiles($release);

        if ($o_queue->is_error()) {
          msg_dialog::display(_("Service infrastructure"), msgPool::siError($o_queue->get_error()), ERROR_DIALOG);
          $this->cache = array();
          return;
        } else {
          if (empty($tmp)) {
            unset($this->cache['SERVERS'][$release]);
          } else {
            $this->cache['PROFILES'][$release] = $tmp;
          }
        }
      }
    }
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"     => _("FAI"),
      "plDescription"   => _("Full automated installation"),
      "plSelfModify"    => FALSE,
      "plPriority"      => 9,
      "plCategory"      => array("workstation", "server", "ogroups"),
      "plObjectType"    => array("workstation", "server", "ogroup"),

      "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }
}

?>
