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

/* TODO
 * Faire une classe qui hérite de Composite attribute et qui gère à la main FAIclass
 * Il faut dans le loadValue mettre les choix dispo grâce au cache.
 *
 * Valider la page si le menu déroulant release change
 * hériter = on appartient à un groupe qui a la classe FAIdlsmfkj et on a pas la valeur (voir quels champs sont obligatoire et comment c'est contourné)
 * remplir le champ dépôt avec ce qu'il faut
 * ajouter auto aux choix.
 *
 * Ajouter un set value hook aux attributs, et faire avec ça pour la maj des choices possibles
 */

class FAIreleaseAttribute extends SelectAttribute
{
  function __construct ($label, $description, $ldapName, $required, $choices, $defaultValue = "", $outputs = NULL, $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, $choices, $defaultValue, $outputs, $acl);
    $this->setSubmitForm(TRUE);
  }

  function setValue ($value)
  {
    if (!in_array($value,$this->choices)) {
      return;
    }
    parent::setValue($value);
    if (isset($this->plugin)) {
      $this->plugin->attributesInfo['main']['attrs']['FAIclass']->attributes[1]->setChoices($this->plugin->cache['PROFILES'][$value]);
      $this->plugin->attributesInfo['main']['attrs']['FAIdebianMirror']->setChoices($this->plugin->cache['SERVERS'][$value]);
      $this->plugin->update_kernel_cache($value);
      $this->plugin->attributesInfo['startup']['attrs']['gotoBootKernel']->setChoices($this->plugin->cache['KERNELS'][$value]);
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

  /* attribute list for save action */
  var $objectclasses        = array("FAIobject");

  var $member_of_ogroup = FALSE;
  var $sdaemon_available;

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    $s_daemon = new supportDaemon();
    $this->sdaemon_available = $s_daemon->is_available();

    $this->update_fai_cache();

    parent::__construct($config, $dn, $object);

    /* Check object group membership */
    if (!isset($this->parent->by_object['ogroup'])) {
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      $ldap->search(
        "(&(objectClass=gotoWorkstationTemplate)(objectClass=FAIobject)(FAIclass=*)(FAIdebianMirror=*)(member=".LDAP::prepare4filter($this->dn)."))",
        array("FAIclass","FAIdebianMirror","gotoBootKernel","gotoKernelParameters")
      );
      if ($ldap->count() == 1) {
        $this->member_of_ogroup = TRUE;
        $attrs = $ldap->fetch();
        $this->o_group_attrs = $attrs;
      }
    }
  }

  function execute()
  {
    $smarty = get_smarty();
    $display = parent::execute();
    if (!$this->displayPlugin && $this->member_of_ogroup) {
      $FAIprofile = $FAIrelease = NULL;
      sscanf($this->o_group_attrs['FAIclass'][0], '%[^ :] :%[^ :]', $FAIprofile, $FAIrelease);
      $smarty->assign("profile", $FAIprofile);
      $smarty->assign("release", $FAIrelease);

      $smarty->assign("mirror", $this->o_group_attrs['FAIdebianMirror'][0]);

      $kernel = (isset($this->o_group_attrs['gotoBootKernel'][0])?$this->o_group_attrs['gotoBootKernel'][0]:"");
      $smarty->assign("kernel", $kernel);

      $kernelParams = (isset($this->o_group_attrs['gotoKernelParameters'][0])?$this->o_group_attrs['gotoKernelParameters'][0]:"");
      $smarty->assign("kernelParams", $kernelParams);

      $display .= $smarty->fetch(get_template_path('faistartup-inheritance.tpl', TRUE, dirname(__FILE__)));
    }
    return $display;
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

  function update_kernel_cache($release)
  {
    /* Get list of available kernel for this release
     */
    if (!isset($this->cache['KERNELS'])) {
      $this->cache['KERNELS'] = array();
    }

    if ($this->sdaemon_available && !isset($this->cache['KERNELS'][$release])) {
      $s_daemon = new supportDaemon();
      if (!$s_daemon->is_available()) {
        $this->sdaemon_available = FALSE;
        return;
      }
      $tmp = $s_daemon->FAI_get_kernels($release);
      if ($s_daemon->is_error()) {
        $this->sdaemon_available = FALSE;
        return;
      }
      $this->cache['KERNELS'][$release] = $tmp;
      ksort($this->cache['KERNELS'][$release]);
    }
  }

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
                _('Release'),
                _('FAI debian release to be installed on this computer'),
                'FAIrelease',
                TRUE,
                array('')
              ),
              new SelectAttribute (
                _('Profil'),
                _('FAI profil to be applied to this computer'),
                'FAIprofile',
                TRUE,
                array('')
              ),
            ),
            '%2$[^ :] :%1$[^ :]',
            '%2$s :%1$s'
          ),
          new SelectAttribute (
            _('Repository'),
            _('FAI Debian repository to be used for installation'),
            'FAIdebianMirror',
            TRUE,
            array('')
          ),
        )
      ),
      'startup' => array(
        'name' => _('Startup parameters'),
        'attrs' => array (
          new SelectAttribute (
            _('Boot kernel'),
            _('Linux kernel to be installed and booted'),
            'gotoBootKernel',
            FALSE,
            array('')
          ),
          new StringAttribute (
            _('Boot kernel parameters'),
            _('Linux kernel parameters to be ran'),
            'gotoKernelParameters',
            FALSE,
            ''
          )
        )
      ),
    );
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"     => _("FAI"),
      "plDescription"   => _("Full automated installation"),
      "plSelfModify"    => FALSE,
      "plPriority"      => 9,
      "plSection"       => array("administration"),
      "plCategory"      => array("workstation", "server", "ogroups"),

      "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }
}

?>
