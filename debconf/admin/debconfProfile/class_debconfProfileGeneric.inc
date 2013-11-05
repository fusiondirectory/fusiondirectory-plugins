<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011-2013  FusionDirectory

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
    parent::__construct ($description, $ldapName, array(), "", "", $acl);
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
    $this->attributes = array();

    /* Load template */
    $ldap = $this->plugin->config->get_ldap_link();
    $ou = $this->plugin->attributesInfo['main']['attrs']['ou']->getValue();
    $ldap->cd("ou=templates,".$this->plugin->dn);
    $ldap->search("(objectClass=debConfDbEntry)");
    while ($entry = $ldap->fetch()) {
      if (isset($entry['cn'][0])) {
        $cn     = $entry['cn'][0];
        $desc   = (isset($entry['description'][0])?$entry['description'][0]:'');
        $type   = (isset($entry['type'][0])?$entry['type'][0]:'');
        $default  = (isset($entry['value'][0])?$entry['default'][0]:'');
        $this->attributes[$cn] = new StringAttribute ($cn, $desc, "debconfentry_$cn", FALSE, $default);
        $this->attributes[$cn]->setInLdap(FALSE);
        $this->attributes[$cn]->setAcl($this->getAcl());
        //~ 'choices'
        //~ 'extendedDescription'
      }
    }
    $ldap->cd("ou=questions,".$this->plugin->dn);
    $ldap->search("(objectClass=debConfDbEntry)");
    while ($entry = $ldap->fetch()) {
      if (isset($entry['cn'][0]) && isset($entry['value'][0])) {
        $cn = $entry['cn'][0];
        if (isset($this->attributes[$cn])) {
          $this->attributes[$cn]->setValue($entry['value'][0]);
          $this->attributes[$cn]->loadValue(NULL); // set initialValue
          //~ 'flags'
          //~ 'owners'
          //~ 'template'
        } else {
          msg_dialog::display(_("Error"),  _("There is no template for this profile"), ERROR_DIALOG);
        }
      }
    }
  }

  function saveInLdap()
  {
    $ldap = $this->plugin->config->get_ldap_link();
    foreach ($this->attributes as $cn => $attribute) {
      $dn = "cn=$cn,ou=questions,".$this->plugin->dn;
      $ldap->cat($dn,array("objectClass"));
      if ($ldap->count() != 1) {
        msg_dialog::display(_("Error"),
          sprintf(_("Can't find entry %s in LDAP for profile %s"), $cn, $this->plugin->dn),
          ERROR_DIALOG);
      } else {
        $attrs = array(
          'value' => array($attribute->getValue()),
        );
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
    parent::__construct ($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setInLdap(FALSE);
  }

  /*! \brief This function read the file from the given handle and then closes it
   *
   *  \param filehandle $handle The handle on the opened uploaded file
   */
  function readFile($handle)
  {
    $matches = array();
    $str = fread($handle, 1024);
    strpos($str, "dn:");
    $tmp = str_replace ("\n", "", $str);  // removing breaklines
    $tmp = str_replace (" ", "", $tmp);   // removing spaces
    if (preg_match( "/dn:.*,ou=templates,ou=(.+),".get_ou("debconfRDN").
                    $this->plugin->config->current['BASE']."/",
                    $tmp, $matches) > 0) {    // searching for dn: containing the template name
      $this->plugin->attributesInfo['main']['attrs']['ou']->setPostValue($matches[1]);
      $this->plugin->attributesInfo['main']['attrs']['ou']->setValue($matches[1]);
      $this->plugin->attributesInfo['main']['attrs']['ou']->setDisabled(TRUE);

      while (!feof($handle)) {
        $str .= fread($handle, 1024);
      }
      @fclose($handle);

      $ldap = $this->plugin->config->get_ldap_link();
      try {
        $ldap->import_complete_ldif($str, FALSE, FALSE);
      } catch(Exception $e) {
        msg_dialog::display(_('LDAP error'), $e->getMessage(), ERROR_DIALOG);
      }
    } else {
      msg_dialog::display(_("LDIF error"),  "Could not found template name in the ldif ".
                                            "was expecting ou=<b>name</b>,ou=templates,".
                                            get_ou("debconfRDN").$this->plugin->config->current['BASE'],
                                            ERROR_DIALOG);
    }
  }

  function renderFormInput()
  {
    return sprintf(_("In order to import a debconf file, please run the following command : <br/>".
        "<i>debconf2ldif.pl -b ou=<b>name</b>,%s -k <b>filename</b> > template.ldif </i><br/>".
        "With <b>filename</b> the file name, and <b>name</b> the desired name for the template.<br/>"),
        get_ou("debconfRDN").$this->plugin->config->current['BASE']).parent::renderFormInput();
  }
}

class debconfProfileGeneric extends simplePlugin
{
  var $mainTab = TRUE;

  var $objectclasses   = array("top","organizationalUnit");

  public function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->attributesInfo['main']['attrs']['ou']->setUnique(TRUE);
    if (count($this->attributesInfo['entries']['attrs']['debconfEntries']->attributes) == 0) {
      unset($this->attributesInfo['entries']);
      $this->attributesInfo['main']['attrs']['ou']->setVisible(FALSE);
      $this->attributesInfo['main']['attrs']['ou']->setDisabled(TRUE);
      $this->attributesInfo['main']['name'] = _('Import');
    } else {
      $this->attributesInfo['main']['attrs']['import']->setVisible(FALSE);
      $this->attributesInfo['main']['attrs']['import']->setDisabled(TRUE);
    }
  }

  function ldap_save($cleanup = TRUE)
  {
    parent::ldap_save($cleanup);
    if (isset($this->attributesInfo['entries'])) {
      $this->attributesInfo['entries']['attrs']['debconfEntries']->saveInLdap();
    }
  }

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      // Attributes are grouped by section
      'main' => array(
        'name'  => _("Name"),
        'attrs' => array(
          new DebconfImportAttribute ("", _("Import a debconf file"), "import"),
          new StringAttribute (_("Name"), _("Name of this debconf template"), "ou", TRUE),
        )
      ),
      'entries' => array(
        'name'  => _("Entries"),
        'attrs' => array(
          new DebconfEntriesAttribute (_("Debconf template answers"), "debconfEntries"),
        )
      ),
    );
  }

  /* Return plugin informations for acl handling  */
  public static function plInfo()
  {
    return array(
      "plShortName"   => _("Debconf profile"),
      "plDescription" => _("Debconf profile generic"),
      "plSelfModify"  => FALSE,
      "plCategory"    => array("debconfProfile" => array("description"  => _("Debconf profile"),
                                                          "objectClass"  => "debConfDbEntry")),
      "plObjectType"  => array("debconfProfile" => array( "name"  => _("Debconf profile"),
                                                          "filter"  => "objectClass=debConfDbEntry")),
      "plProvidedAcls" => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }
}
?>
