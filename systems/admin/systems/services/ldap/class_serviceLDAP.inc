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

class serviceLDAP extends simpleService
{
  /* This plugin only writes its objectClass */
  var $objectclasses    = array("goLdapServer");

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    global $config;
    return array(
      // Attributes are grouped by section
      'main' => array(
        'name'  => _('LDAP service'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new StringAttribute (
            _('LDAP URI'), _('The LDAP URI to use in order to contact this LDAP server'),
            'goLdapBase', TRUE,
            'ldap://'._('fill-in-your-servers-dns-name').':389/'.$config->current['BASE'],
            '',
            "/^ldap[si]?:\/\/[0-9a-z_.-]+(:[0-9]+)?\/.+$/"
          ),
        )
      ),
    );
  }

  protected function ldap_save ($cleanup = TRUE)
  {
    parent::ldap_save($cleanup);

    /* Update object relation */
    if ($this->attributesInfo['main']['attrs']['goLdapBase']->hasChanged()) {
      $oldValue = $this->attributesInfo['main']['attrs']['goLdapBase']->getInitialValue();
      $newValue = $this->attributesInfo['main']['attrs']['goLdapBase']->getValue();

      $new = array();
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      $ldap->search('(&(objectClass=GOhard)(gotoLdapServer=*'.$oldValue.'))', array("cn","gotoLdapServer"));
      while ($attrs = $ldap->fetch()) {
        $ldapServer = array();
        for ($i = 0; $i < $attrs['gotoLdapServer']['count']; $i++) {
          $ldapServer[] = preg_replace("/".preg_quote($oldValue, '/')."/", $newValue, $attrs['gotoLdapServer'][$i]);
        }
        $new[$attrs['dn']]['gotoLdapServer'] = $ldapServer;
      }
      foreach ($new as $dn => $data) {
        $ldap->cd($dn);
        $ldap->modify($data);
        if (!$ldap->success()) {
          msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
        }
      }
    }
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("LDAP service"),
      "plDescription" => _("LDAP")." ("._("Services").")",
      "plPriority"    => 30,
      "plCategory"    => array("server"),

      "plProvidedAcls" => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }
}
?>
