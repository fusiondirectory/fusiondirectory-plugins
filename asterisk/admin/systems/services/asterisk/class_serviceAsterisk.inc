<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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

class serviceAsterisk extends simpleService {

  /* This plugin only writes its objectClass */
  var $objectclasses    = array("goFonServer");

  /* This class can't be assigned twice so it conflicts with itsself */
  var $conflicts        = array();

  static function getAttributesInfo ()
  {
    return array(
      'database_information' => array(
        'name'  => _("VoIP database information"),
        'attrs' => array(
          new StringAttribute (
            _("Asterisk DB user"), _("Login for asterisk database administrator"),
            "goFonAdmin", TRUE
          ),
          new PasswordAttribute (
            _("Password"), _("Password for asterisk database administrator"),
            "goFonPassword", TRUE
          ),
          new StringAttribute (
            _("Country dial prefix"), _("Country dial prefix"),
            "goFonCountryCode", TRUE
          ),
          new StringAttribute (
            _("Local dial prefix"), _("Local dial prefix"),
            "goFonAreaCode", TRUE
          ),
          new SelectAttribute (
            _('Driver'), _('Driver to use to contact this database.'),
            'fdAsteriskDriver', TRUE
          ),
        )
      ),
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $drivers = array(
      'fbsql','ibase','mssql','mysql','mysqli','oci8','odbc','pgsql','querysim','sqlite','sqlsrv'
    );
    sort($drivers);
    $this->attributesAccess['fdAsteriskDriver']->setChoices($drivers);
  }

  function allow_remove()
  {
    /* Check if we are able to remove the asterisk database. If the database is still in use skip */
    if ($this->is_account) {
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      $ldap->search("(&(goFonHomeServer=".$this->dn.")(|(uid=*)(cn=*)))", array("dn","uid","cn","goFonHomeServer"));

      if ($ldap->count()) {
        /* Number of entries shown in warning */
        $i    = 3;
        $str  = "";
        while (($attrs = $ldap->fetch()) && $i >= 0) {
          $i--;
          if (isset($attrs['uid'][0])) {
            $str .= $attrs['uid'][0]." ";
          } else {
            $str .= $attrs['cn'][0]." ";
          }
        }

        /* Some entries found */
        if ($i != 3) {
          return sprintf(_("You can't remove the asterisk database extension, it is still in use by these objects '%s'."), trim($str));
        }
      }
    }
  }

  function save ()
  {
    parent::save();

    /* Update goFonAccounts if server was is renamed.
       Set attribute goFonHomeServer to current dn
     */
    if ($this->is_account) {
      if ($this->orig_dn != "new" && $this->orig_dn != $this->dn) {
        $ldap = $this->config->get_ldap_link();
        $ldap->cd($this->config->current['BASE']);
        $ldap->search("(&(goFonHomeServer=".$this->orig_dn.")(|(objectClass=goFonAccount)(objectClass=goFonConference)))", array("goFonHomeServer"));
        $dns = array();
        while ($attrs = $ldap->fetch()) {
          $dns[] = $attrs['dn'];
        }
        foreach ($dns as $dn) {
          $ldap->cd($dn);
          $ldap->modify(array("goFonHomeServer" => $this->dn));
          if (!$ldap->success()) {
            msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $dn, LDAP_MOD, get_class()));
          }
        }
      }

      /* Updated config list of gofon servers */
      if ($this->orig_dn == "new" || $this->orig_dn != $this->dn) {
        $config = session::get('config');
        $config->load_servers();
        session::set('config', $config);
      }
    }
  }


  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"     => _("VoIP service (asterisk)"),
      "plDescription"   => _("VoIP - asterisk management")." ("._("Services").")",
      "plIcon"          => "plugins/asterisk/images/iconMini.png",
      "plPriority"      => 92,

      "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }
}
?>
