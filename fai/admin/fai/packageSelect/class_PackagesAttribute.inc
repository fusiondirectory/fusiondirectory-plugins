<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2019  FusionDirectory

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

class PackagesAttribute extends DialogOrderedArrayAttribute
{
  protected $dialogClass        = 'PackageSelectManagementDialog';
  protected $buffer             = [];
  protected $configuredPackages = [];

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = [], $acl = "")
  {
    OrderedArrayAttribute::__construct(
      new StringAttribute($label, $description, $ldapName, $required, "", $acl),
      FALSE,
      $defaultValue,
      FALSE
    );
  }

  public function getSelectManagementParameters (): array
  {
    /*
     * objectTypes
     * multiSelect
     * neededAttrs
     * blackList
     * whiteList
     * filterElementDefinitions
     * */
    return [
      $this->plugin,
      ['faiRepositoryPackage'],
      TRUE,
      [],
      $this->getFilterBlackList(),
      [],
      []
    ];
  }

  function addValue (string $dn, $entry = NULL)
  {
    $this->value[preg_replace('/\-$/', '', $entry['PACKAGE'][0])] = $entry['PACKAGE'][0];
  }

  function readValue ($value)
  {
    return [preg_replace('/\-$/', '', $value), $value];
  }

  protected function sortValues ()
  {
    /* Keep key => value association */
    asort($this->value);
  }

  function getFilterBlackList ()
  {
    return ['PACKAGE' => array_keys($this->value)];
  }

  function renderButtons ()
  {
    $id = $this->getHtmlId();
    $buttons  = '<input type="submit" formnovalidate="formnovalidate" name="add'.$id.'" id="add'.$id.'"'.
                ($this->disabled ? ' disabled="disabled"' : '').
                ' value="{msgPool type=addButton}"/>';
    $buttons  .= '<input type="submit" formnovalidate="formnovalidate" name="add'.$id.'_dialog" id="add'.$id.'_dialog"'.
                ($this->disabled ? ' disabled="disabled"' : '').
                ' value="{msgPool type=addButton}{t} from list{/t}"/>';
    return $buttons;
  }

  public function htmlIds (): array
  {
    $id = $this->getHtmlId();
    return array_merge(['add'.$id], parent::htmlIds());
  }

  protected function getAttributeArrayValue ($key, $value)
  {
    if (!isset($this->buffer[$value])) {
      $this->genPkgs();
    }
    if (isset($this->buffer[$value])) {
      return [
        $this->buffer[$value]['PACKAGE'],
        $this->buffer[$value]['VERSION'],
        base64_decode($this->buffer[$value]['DESCRIPTION']),
      ];
    }
    return [$value, '', ''];
  }

  protected function genRowIcons ($key, $value)
  {
    $id = $this->getHtmlId();
    list ($img, $nbicons) = parent::genRowIcons($key, $value);
    $key64 = base64_encode($key);
    if (preg_match('/\-$/', $value)) {
      $img = $this->renderInputField(
        'image', $id.'_removal_'.$key64,
        [
          'src'             => 'geticon.php?context=actions&icon=add&size=16',
          'title'           => _('Mark for installation'),
          'alt'             => _('Mark for installation'),
          'formnovalidate'  => 'formnovalidate',
          'class'           => 'center'
        ],
        FALSE
      ).$img;
    } else {
      $img = $this->renderInputField(
        'image', $id.'_removal_'.$key64,
        [
          'src'             => 'geticon.php?context=actions&icon=remove&size=16',
          'title'           => _('Mark for removal'),
          'alt'             => _('Mark for removal'),
          'formnovalidate'  => 'formnovalidate',
          'class'           => 'center'
        ],
        FALSE
      ).$img;
    }
    if (isset($this->buffer[$key]['HASTEMPLATE']) && $this->buffer[$key]['HASTEMPLATE']) {
      $img = $this->renderInputField(
        'image', $id.'_config_'.$key64,
        [
          'src'             => 'geticon.php?context=actions&icon=package-configure&size=16',
          'title'           => _('Edit configuration'),
          'alt'             => _('Edit configuration'),
          'formnovalidate'  => 'formnovalidate',
          'class'           => 'center'
        ],
        FALSE
      ).$img;
    } else {
      $img = '<img src="images/empty.png" alt="" style="width:16px;"/>'.$img;
    }
    $nbicons += 2;

    return  [$img, $nbicons];
  }

  protected function handleAddAndEditValue ()
  {
    $id = $this->getHtmlId();

    foreach (array_keys($_POST) as $name) {
      if (preg_match('/^'.$id.'_config_/', $name)) {
        $key = preg_replace('/^'.$id.'_config_/', '', $name);
        $key = base64_decode(preg_replace('/_[xy]$/', '', $key));
        /* Open configuration dialog */
        $pkg_config = [];
        if (isset($this->configuredPackages[$key])) {
          $pkg_config = $this->configuredPackages[$key];
        }
        $this->plugin->openDialog(
          new PackageConfigDialog($this->plugin, $this, $key, $pkg_config)
        );
        break;
      } elseif (preg_match('/^'.$id.'_removal_/', $name)) {
        $key = preg_replace('/^'.$id.'_removal_/', '', $name);
        $key = base64_decode(preg_replace('/_[xy]$/', '', $key));
        if (preg_match('/\-$/', $this->postValue[$key])) {
          $this->postValue[$key] = preg_replace('/\-$/', '', $this->postValue[$key]);
        } else {
          $this->postValue[$key] .= '-';
        }
        break;
      }
    }

    parent::handleAddAndEditValue();
  }

  function packageConfigured ($packageConfig)
  {
    $this->configuredPackages = array_merge($this->configuredPackages, $packageConfig);
  }

  /* Special LDAP treatment that this attribute does after plugin ldap save */
  function postLdapSave ($ldap)
  {
    /* First delete all old nodes */
    $ldap->cd($this->plugin->dn);
    $ldap->search('objectClass=FAIdebconfInfo', ['dn'], 'one');
    $delete = [];
    while ($attrs = $ldap->fetch()) {
      $delete[] = $attrs['dn'];
    }
    foreach ($delete as $dn) {
      $ldap->rmdir($dn);
    }

    /* Then add our values */
    foreach ($this->configuredPackages as $pkgname => $pkgvars) {
      foreach ($pkgvars as $varname => $varinfos) {
        $attrs = ['objectClass' => 'FAIdebconfInfo'];
        $attrs['FAIpackage']           = $pkgname;
        $attrs['FAIvariable']          = $varname;
        $attrs['FAIvariableType']      = $varinfos['Type'];
        $attrs['FAIvariableContent']   = $varinfos['Value'];
        $vardn = 'FAIvariable='.$varname.','.$this->plugin->dn;

        $ldap->cd($vardn);
        foreach (array_keys($attrs) as $index) {
          if (is_array($attrs[$index]) && (count($attrs[$index]) == 0)) {
            unset($attrs[$index]);
          }
        }
        $ldap->add($attrs);
        if (!$ldap->success()) {
          $error = new SimplePluginLdapError($this, $vardn, LDAP_ADD, $ldap->get_error(), $ldap->get_errno());
          $error->display();
        }
      }
    }
  }

  /*! \brief  Reload the list of cached packages.
      @return Returns the currently cached list of packages.
   */
  function genPkgs ()
  {
    if (empty($this->plugin->FAIdebianRelease)) {
      return;
    }

    if (empty($this->value)) {
      // If there are no packages, no need to do anything
      return;
    }

    $attrs = ['distribution','package','version','section','description','timestamp','hastemplate'];

    // packages names that are not already in the buffer
    $packages = array_diff_key($this->value, $this->buffer);

    $release  = $this->plugin->FAIdebianRelease;
    $q        = new supportDaemon();
    $customs  = serviceRepository::getCustomReleases($release);
    for ($i = -1; (count($packages) > 0) && ($i < count($customs)); $i++) {
      // if release has been handled
      if ($i >= 0) {
        // take the next custom
        $release = $customs[$i];
      }
      $ret = $q->FAI_get_packages($release, $attrs, array_keys($packages), -1, -1, TRUE);
      if ($q->is_error()) {
        $error = new FusionDirectoryError(msgPool::siError($q->get_error()));
        $error->display();
        return $this->buffer;
      } else {
        foreach ($ret as $attr) {
          if (!isset($attr['DESCRIPTION'])) {
            $attr['DESCRIPTION'] = '';
          }
          $this->buffer[$attr['PACKAGE']] = $attr;
          // remove found packages
          unset($packages[$attr['PACKAGE']]);
        }
      }
      @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, count($packages), "$release done, packages left");
    }

    if (count($packages) > 0) {
      $error = new FusionDirectoryError(
        htmlescape(sprintf(
          _('The following packages were not found on the repository : %s'),
          join(',', $packages)
        ))
      );
      $error->display();
      foreach ($packages as $p => $pp) {
        unset($this->value[$p]);
      }
    }

    $this->genConfiguredPackages();
  }

  function genConfiguredPackages ()
  {
    global $config;
    /* Fetch all package configurations from ldap */
    $PackageFilter = "";
    foreach ($this->value as $name) {
      $PackageFilter .= "(FAIpackage=".$name.")";
    }
    /* Also fetch d-i */
    $PackageFilter .= "(FAIpackage=d-i)";

    $PackageFilter = "(&(objectClass=FAIdebconfInfo)(|".$PackageFilter."))";

    /* Search for configuration objects */
    $ldap = $config->get_ldap_link();
    $ldap->cd($this->plugin->dn);
    $ldap->search($PackageFilter, ["FAIvariable","FAIvariableType",
          "FAIvariableContent","FAIpackage","FAIdebianSection","FAIstate"]);

    $this->configuredPackages = [];

    /* Walk through configurations and append them to our list of ConfiguredPackages */
    while ($attr = $ldap->fetch()) {
      /* Skip objects, that are tagged as removed */
      if (isset($attr['FAIstate'][0])) {
        if (preg_match("/removed$/", $attr['FAIstate'][0])) {
          continue;
        }
      }

      $tmp = [];
      $tmp['Name']  = $attr['FAIvariable'][0];
      $tmp['Type']  = $attr['FAIvariableType'][0];
      $tmp['Save']  = TRUE;

      if (isset($attr['FAIvariableContent'][0])) {
        if (!in_array($attr['FAIvariableType'], ["multiselect"])) {
          $tmp['Value'] = $attr['FAIvariableContent'][0];
        } else {
          $tmp['Value'] = [];
          unset($attr['FAIvariableContent']['count']);
          foreach ($attr['FAIvariableContent'] as $attr) {
            $tmp['Value'][] = $attr;
          }
        }
        $this->configuredPackages[$attr['FAIpackage'][0]][$attr['FAIvariable'][0]] = $tmp;
      } else {
        $tmp['Value']                                                               = "";
        $this->configuredPackages[$attr['FAIpackage'][0]][$attr['FAIvariable'][0]]  = $tmp;
      }
    }
  }
}
