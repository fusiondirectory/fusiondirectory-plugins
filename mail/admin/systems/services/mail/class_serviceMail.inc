<?php

class serviceMail extends simpleService{

  /* This plugin only writes its objectClass */
  var $objectclasses    = array("goMailServer");
  var $StatusFlag       = "goMailServerStatus";
  var $goMailServerStatus = "";

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"    => _("Mail smtp (Postfix)"),
          "plDescription"  => _("Mail smtp - Postfix")." ("._("Services").")",
          "plSelfModify"   => FALSE,
          "plDepends"      => array(),
          "plPriority"     => 98,
          "plSection"      => array("administration"),
          "plCategory"     => array("server"),
          "plProvidedAcls" => parent::generatePlProvidedAcls(self::getAttributesInfo())
            ));
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
          new StringAttribute (
            _('Visible full qualified hostname'),
            _('The full qualified host name.'),
            'postfixMyHostname',
            TRUE
          ),
          new StringAttribute (
            _('Visible full qualified domain'),
            _('The full qualified domain.'),
            'postfixMyDomain',
            TRUE
          ),
          new IntAttribute (
            _('Max mail header size').' (KB)',
            _('This value specifies the maximal header size.'),
            'postfixHeaderSizeLimit',
            FALSE,FALSE,FALSE,0
          ),
          new IntAttribute (
            _('Max mailbox size').' (KB)',
            _('Defines the maximal size of mail box.'),
            'postfixMailboxSizeLimit',
            FALSE,FALSE,FALSE,0
          ),
          new IntAttribute (
            _('Max message size').' (KB)',
            _('Specify the maximal size of a message.'),
            'postfixMessageSizeLimit',
            FALSE,FALSE,FALSE,0
          ),
          new StringAttribute (
            _('Relay host'),
            _('Relay messages to following host'),
            'postfixRelayhost'
          ),
          new SetAttribute (
            new StringAttribute (
            _('Local networks'),
            _('Postfix networks'),
            'postfixMyNetworks'
            )
          )
        )
      ),
      'section2' => array (
        'name' => _('Domains and routing'),
        'attrs' => array (
          new SetAttribute (
            new StringAttribute (
            _('Domains to accept mail for'),
            _('Postfix networks'),
            'postfixMyDestinations'
            )
          ),
          new SetAttribute (
            new CompositeAttribute (
             _('Transport'),
            'postfixTransportTable',
              array(
                new StringAttribute ('', '','first'),
                new SelectAttribute ('', '','filter', FALSE, array('smtp')),
                new StringAttribute ('', '','second'),
              ),
              '%s %s:%s',
              '%s %s:%s',
              '', _('Transport')
            )
          )
        )
      ),
      'section3' => array (
        'name' => _('Restrictions'),
        'attrs' => array (
          new SetAttribute (
            new CompositeAttribute (
             _('Restrictions for sender'),
            'postfixSenderRestrictions',
              array(
                new StringAttribute ('', '','first2'),
                new SelectAttribute ('', '','filter2', FALSE, array('FILTER')),
                new StringAttribute ('', '','second2'),
              ),
              '%s %s %s',
              '%s %s %s',
              '', _('Restrictions for sender')
            )
          ),
          new SetAttribute (
            new CompositeAttribute (
             _('Restrictions for recipient'),
            'postfixRecipientRestrictions',
              array(
                new StringAttribute ('', '','first3'),
                new SelectAttribute ('', '','filter3', FALSE, array('FILTER')),
                new StringAttribute ('', '','second3'),
              ),
              '%s %s %s',
              '%s %s %s',
              '', _('Restrictions for recipient')
            )
          )
        )
      ),
      'section4' => array (
        'name' => _('Action'),
        'icon' => 'images/rocket.png',
        'attrs' => array (
          new SelectAttribute (
            _('Status'), '','flagmail', FALSE, array(SERVICE_STOPPED=>SERVICE_STOPPED,SERVICE_STARTED => SERVICE_STARTED,SERVICE_RESTARTED=>SERVICE_RESTARTED)
          )
        )
      )
    );
  }

   /*!
   * \brief Constructor of the freeradiusAccount
   *        It will build the select groups choices
   */
  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $this->attributesInfo['section1']['attrs']['postfixMyDomain']->setDisabled(TRUE);
    $this->attributesInfo['section4']['attrs']['flagmail']->setInLdap(FALSE);
    $goMailServerStatus = $this->attributesInfo['section4']['attrs']['flagmail']->getValue();
  }

  function save()
  {
    $this->prepare_save();
    $this->attributesInfo['section1']['attrs']['postfixMyDomain']->setValue($this->attributesInfo['section1']['attrs']['postfixMyDomain']->getValue());
    $this->ldap_save();
  }

}
?>
