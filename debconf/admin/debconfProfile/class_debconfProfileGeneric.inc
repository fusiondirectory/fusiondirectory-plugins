<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
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

class DebconfEntriesAttribute extends CompositeAttribute
{
  function __construct ($description, $ldapName, $acl = "")
  {
    parent::__construct($description, $ldapName, [], "", "", $acl);
    $this->setInLdap(FALSE);
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      $this->loadEntries();
    }
  }

  function loadEntries ()
  {
    global $config;
    $this->attributes = [];

    /* Load template */
    $ldap = $config->get_ldap_link();
    $ldap->cd("ou=templates,".$this->plugin->dn);
    $ldap->search("(objectClass=debConfDbEntry)");
    while ($entry = $ldap->fetch()) {
      if (isset($entry['cn'][0])) {
        $cn     = $entry['cn'][0];
        $desc   = (isset($entry['description'][0]) ? $entry['description'][0] : '');
        $default  = (isset($entry['value'][0]) ? $entry['default'][0] : '');
        $this->attributes[$cn] = new StringAttribute($cn, $desc, "debconfentry_$cn", FALSE, $default);
        $this->attributes[$cn]->setInLdap(FALSE);
        $this->attributes[$cn]->setAcl($this->getAcl());
        /* Ignored fields: choices,extendedDescription */
      }
    }
    $ldap->cd("ou=questions,".$this->plugin->dn);
    $ldap->search("(objectClass=debConfDbEntry)");
    while ($entry = $ldap->fetch()) {
      if (isset($entry['cn'][0]) && isset($entry['value'][0])) {
        $cn = $entry['cn'][0];
        if (isset($this->attributes[$cn])) {
          $this->attributes[$cn]->setValue($entry['value'][0]);
          // set initialValue
          $this->attributes[$cn]->loadValue([]);
          /* Ignored fields: flags,owners,template */
        } else {
          $error = new SimplePluginError($this, htmlescape(_('There is no template for this profile')));
          $error->display();
        }
      }
    }
  }

  function saveInLdap ()
  {
    global $config;
    $ldap = $config->get_ldap_link();
    foreach ($this->attributes as $cn => $attribute) {
      $dn = "cn=$cn,ou=questions,".$this->plugin->dn;
      $ldap->cat($dn, ["objectClass"]);
      if ($ldap->count() != 1) {
        $error = new SimplePluginError(
          $this,
          htmlescape(sprintf(
            _('Can\'t find entry %s in LDAP for profile %s'),
            $cn,
            $this->plugin->dn
          ))
        );
        $error->display();
      } else {
        $attrs = [
          'value' => [$attribute->getValue()],
        ];
        $ldap->cd($dn);
        $ldap->modify($attrs);
      }
    }
  }
}

class DebconfImportAttribute extends FileAttribute
{
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = "", $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setInLdap(FALSE);
  }

  /*! \brief This function read the file from the given handle and then closes it
   *
   *  \param filehandle $handle The handle on the opened uploaded file
   */
  function readFile ($handle)
  {
    global $config;
    $matches = [];
    $str = fread($handle, 1024);
    // removing breaklines
    $tmp = str_replace("\n", "", $str);
    // removing spaces
    $tmp = str_replace(" ", "", $tmp);
    if (preg_match("/dn:.*,ou=templates,ou=([^,=]+),".get_ou("debconfRDN").
                    $config->current['BASE']."/",
                    // searching for dn: containing the template name
                    $tmp, $matches) > 0) {
      $this->plugin->attributesAccess['ou']->setPostValue($matches[1]);
      $this->plugin->attributesAccess['ou']->setValue($matches[1]);
      $this->plugin->attributesAccess['ou']->setDisabled(TRUE);

      while (!feof($handle)) {
        $str .= fread($handle, 1024);
      }
      @fclose($handle);

      $ldap = $config->get_ldap_link();
      try {
        $ldap->import_complete_ldif($str, FALSE, FALSE);
        $this->plugin->initially_was_account = TRUE;
      } catch (FusionDirectoryException $e) {
        $error = new SimplePluginError($this, htmlescape($e->getMessage()), 0, $e);
        $error->display();
      }
    } else {
      $error = new SimplePluginError(
        $this,
        "Could not find template name in the ldif ".
          "was expecting ou=<b>name</b>,ou=templates,".
          htmlescape(get_ou("debconfRDN").$config->current['BASE'])
      );
      $error->display();
    }
  }

  function renderFormInput (): string
  {
    global $config;
    return sprintf(_("In order to import a debconf file, please run the following command : <br/>".
        "<i>debconf2ldif.pl -b ou=<b>name</b>,%s -k <b>filename</b> > template.ldif </i><br/>".
        "With <b>filename</b> the file name, and <b>name</b> the desired name for the template.<br/>"),
        get_ou("debconfRDN").$config->current['BASE']).parent::renderFormInput();
  }
}

class debconfProfileGeneric extends simplePlugin
{
  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('Debconf profile'),
      'plDescription' => _('Debconf profile information'),
      'plObjectClass' => ['organizationalUnit'],
      'plFilter'      => '(&(objectClass=organizationalUnit)(!(objectClass=gosaDepartment)))',
      'plObjectType'  => ['debconfProfile' => [
        'name'      => _('Debconf profile'),
        'ou'        => get_ou('debconfRDN'),
        'icon'      => 'geticon.php?context=applications&icon=debconf&size=16',
        'mainAttr'  => 'ou',
      ]],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      // Attributes are grouped by section
      'main' => [
        'name'  => _("Name"),
        'attrs' => [
          new DebconfImportAttribute("", _("Import a debconf file"), "import"),
          new StringAttribute(_("Name"), _("Name of this debconf template"), "ou", TRUE),
        ]
      ],
      'entries' => [
        'name'  => _("Entries"),
        'attrs' => [
          new DebconfEntriesAttribute(_("Debconf template answers"), "debconfEntries"),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['ou']->setUnique('one');
    if (count($this->attributesAccess['debconfEntries']->attributes) == 0) {
      unset($this->attributesInfo['entries']);
      $this->attributesAccess['ou']->setVisible(FALSE);
      $this->attributesAccess['ou']->setDisabled(TRUE);
      $this->attributesInfo['main']['name'] = _('Import');
    } else {
      $this->attributesAccess['import']->setVisible(FALSE);
      $this->attributesAccess['import']->setDisabled(TRUE);
    }
  }

  function ldap_save (): array
  {
    $errors = parent::ldap_save();
    if (!empty($errors)) {
      return $errors;
    }
    if (isset($this->attributesInfo['entries'])) {
      $this->attributesInfo['entries']['attrs']['debconfEntries']->saveInLdap();
    }
    return [];
  }
}
