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

class RelayHostKolabAttribute extends CompositeAttribute
{
  function __construct ($description, $ldapName, $acl = "", $label = "Composite attribute")
  {
    $attributes = array(
      new BooleanAttribute (
        _('Enable MX lookup for relayhost'), '', 'RelayMxSupport', FALSE, FALSE,
        "", TRUE, FALSE
      ),
      new StringAttribute (_('Relay host'), _('Host used to relay mails'), 'relayhost')
    );
    parent::__construct ($description, $ldapName, $attributes, "", "", $acl, $label);
  }

  function readValues($value)
  {
    $values = array();
    $values[0] = preg_match('/^\[/', $value);
    $values[1] = preg_replace("/[\[\]]/", "", $value);
    return $values;
  }

  function writeValues($values)
  {
    /* Adapt relayhost */
    $value = preg_replace('/[\[\]]/', '', $values[1]);
    if ($values[0] && ($value != "")) {
      $value = "[".$value."]";
    }
    return $value;
  }
}

class serviceKolab extends simpleService
{
  var $objectclasses  = array("top", "kolab");
  var $displayHeader  = FALSE;
  var $mainTab        = TRUE;

  var $conflicts    = array("goMailServer","GOhard");

  static function plInfo()
  {
    return array(
      "plShortName"     => _("Kolab mail service"),
      "plDescription"   => _("Kolab mail service")." ("._("Services").")",
      "plSelfModify"    => FALSE,
      "plCategory"      => array("server"),
      "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }


  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array (
      'section1' => array (
        'name'  => _('Generic'),
        'icon' => 'images/rightarrow.png',
        'attrs' => array (
          new StringAttribute('', '', 'k', FALSE, 'kolab'),
          new SetAttribute(new StringAttribute('', '', 'kolabHost')),
          new StringAttribute(
            _('Postfix mydomain'), _('Postfix domain name'),
            'postfix-mydomain'
          ),
          new StringAttribute(
            _('Cyrus admins'), _('Cyrus admins for kolab server'),
            'cyrus-admins'
          ),
          new SetAttribute(
            new StringAttribute(
              _('Mail domains').', '._('Postfix mydestination'), _('Mail domains'),
              'postfix-mydestination', FALSE
            )
          )
        )
      ),
      'section2' => array (
        'name'  => _('Services'),
        'attrs' => array (
          new BooleanAttribute (
            _('POP3 service'), _('Enable POP3 service'),
            'cyrus-pop3', FALSE, FALSE
          ),
          new BooleanAttribute (
            _('POP3/SSL service'), _('Enable POP3/SSL service'),
            'cyrus-pop3s', FALSE, TRUE
          ),
          new BooleanAttribute (
            _('IMAP service'), _('Enable IMAP service'),
            'cyrus-imap', FALSE, TRUE
          ),
          new BooleanAttribute (
            _('IMAP/SSL service'), _('Enable IMAP/SSL service'),
            'cyrus-imaps', FALSE, TRUE
          ),
          new BooleanAttribute (
            _('Sieve service'), _('Enable Sieve service'),
            'cyrus-sieve', FALSE, TRUE
          ),
          new BooleanAttribute (
            _('FTP FreeBusy service (legacy)'),
            _('FTP FreeBusy service (legacy, not interoperable with Kolab2 FreeBusy)'),
            'proftpd-ftp', FALSE, FALSE
          ),
          new BooleanAttribute (
            _('HTTP FreeBusy service (legacy)'), '',
            'apache-http', FALSE, TRUE
          ),
          new BooleanAttribute (
            _('Amavis email scanning'), _('Amavis email scanning (virus/spam)'),
            'postfix-enable-virus-scan', FALSE, TRUE
          ),
        )
      ),
      'section3' => array (
        'name' => _('Quota settings'),
        'attrs' => array (
          new IntAttribute (
            _('Quota occupation limit before warning users (%)'),
            _('Warn users when using more than X% of their mail quota'),
            'cyrus-quotawarn', FALSE,
            FALSE, FALSE, // min and max
            80 // default value
          )
        )
      ),
      'section4' => array (
        'name' => _('Free/Busy settings'),
        'attrs' => array (
          new BooleanAttribute (
            _('Allow unauthenticated downloading of Free/Busy information'),
            _('Allow unauthenticated downloading of Free/Busy information'),
            'apache-allow-unauthenticated-fb', FALSE, TRUE
          ),
          new IntAttribute (
            _('Number of days to include when creating free/busy lists'),
            _('Include data from X days in the past when creating free/busy lists'),
            'kolabFreeBusyFuture', FALSE,
            FALSE, FALSE, // min and max
            1 // default value
          )
        )
      ),
      'section5' => array (
        'name' => _('SMTP privileged networks'),
        'attrs' => array (
          new StringAttribute (
            _('Hosts/networks allowed to relay'),
            _('Hosts/networks allowed to relay (multiple values comma-seperated)'),
            'postfix-mynetworks',
            FALSE,
            '127.0.0.1/8'
          )
        )
      ),
      'section6' => array (
        'name' => _('SMTP smarthost/relayhost'),
        'attrs' => array (
          new RelayHostKolabAttribute (
            _('Host used to relay mails'), 'postfix-relayhost'
          )
        )
      ),
      'section7' => array (
        'name' => _('Accept Internet Mail'),
        'attrs' => array (
          new BooleanAttribute (
            _('Accept mail from other domains over non-authenticated SMTP'),
            _('Accept mail from other domains over non-authenticated SMTP'),
            'postfix-allow-unauthenticated'
          )
        )
      )
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    $this->hostname = preg_replace('/^cn=([^,]+),.*$/', '\1', $dn);
    $this->dn       = "k=kolab,".$config->current['BASE'];
    parent::__construct($config, $this->dn);

    $this->attributesInfo['section1']['attrs']['k']->setVisible(FALSE);
    $this->attributesInfo['section1']['attrs']['kolabHost']->setVisible(FALSE);

    $this->is_account = in_array($this->hostname, $this->attributesInfo['section1']['attrs']['kolabHost']->getValue());

    $this->initially_was_account = $this->is_account;

    $this->updateAttributesValues();
  }

  function save ()
  {
    $this->dn = "k=kolab,".$this->config->current['BASE'];

    /* Check if this server was renamed, in this case we have to remove old cn first*/
    $kolabHost = $this->attributesInfo['section1']['attrs']['kolabHost']->getValue();
    if ($this->hostname != $this->cn) {
      $key = array_search($this->hostname, $kolabHost);
      if ($key !== FALSE) {
        unset($kolabHost[$key]);
      }
    }
    if (!in_array($this->cn, $kolabHost)) {
      $kolabHost[] = $this->cn;
    }
    $this->attributesInfo['section1']['attrs']['kolabHost']->setValue($kolabHost);

    parent::save();
  }

  function remove_from_parent()
  {
    /* Only walk through following code, if this host was a member of the kolab hosts, else skip this */
    if (!$this->initially_was_account) {
      return;
    }

    /* !!! Don't use "cn" in this function
       hostname -> the initial name of the host
       cn       -> is the new name of the host, in case that it was renamed.
     */
    $ldap     = $this->config->get_ldap_link();
    $this->dn = "k=kolab,".$this->config->current['BASE'];

    /* We can't simply remove the whole entry, it is possible that there are
       some other hosts assigned to this object.
       So, first of all check if we are the last host entry within the host
       : Remove k=kolab entry
       if we aren't alone, only remove host name from hosts entry and save */
    /* Are we alone? Remove complete entry... */

    $kolabHost = $this->attributesInfo['section1']['attrs']['kolabHost']->getValue();

    /* Check if we are definitly in kolabHosts */
    if (!in_array_ics($this->hostname, $kolabHost)) {
      return;
    }

    /* Integration check, not translated because they can't pop up at all, only for debug */
    if (count($kolabHost) == 0) {
      msg_dialog::display(_("Internal error"), _("Kolab account does not exist!"), ERROR_DIALOG);
      return;
    }
    if (!isset($this->hostname) || (empty($this->hostname))) {
      msg_dialog::display(_("Error"), msgPool::required(_("hostname")), ERROR_DIALOG);
    }

    /* Our hostname is in kolabHosts and there is a only one entry
       = we are the last host entry, delete k=kolab entry  */
    if (count($this->kolabHost) == 1) {
      /* Remove complete entry */
      $ldap->rmdir ($this->dn);
      if (!$ldap->success()) {
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
      }
    } else {
      /* Only modify kolabHost */
      $key = array_search($this->hostname, $kolabHost);
      unset($kolabHost[$key]);
      $attrs = array('kolabHost' => array_values($kolabHost));
      $ldap->cd($this->dn);
      $this->cleanup();
      $ldap->modify($attrs);
      if (!$ldap->success()) {
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()));
      }
    }

    new log("remove", "service/".get_class($this), $this->dn, array_keys($this->attrs), $ldap->get_error());

    /* Optionally execute a command after we're done */
    $this->handle_post_events("remove");
  }

}
?>
