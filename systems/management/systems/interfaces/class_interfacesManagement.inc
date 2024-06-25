<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2020  FusionDirectory

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

class interfacesManagement extends management implements SimpleTab
{
  protected $skipCpHandler     = TRUE;

  public static $skipSnapshots = TRUE;

  public static $skipTemplates = TRUE;

  protected $skipConfiguration = TRUE;

  /* Vars needed for simpleTabs */
  public $is_account      = TRUE;
  public $dn;
  public $is_template     = FALSE;
  public $parent          = NULL;
  public $acl_category    = '';

  /* We store all interfaces opened objects in this array */
  protected $interfaces   = [];

  var $read_only      = FALSE;
  var $acl;
  var $cn;

  public $attrs = [];

  /* Used by template and webservice */
  public $attributesAccess;

  /* We list all network interfaces attributes in neededAttrs to get all data */
  public $neededAttrs = ['cn','macAddress','fdNetworkInterfaceVlanDN','fdNetworkInterfaceVlanTagged','fdNetworkInterfaceSubnetDN','ipHostNumber'];

  public static $columns = [
    ['LinkColumn',           ['attributes' => 'nameAttr',                     'label' => 'Name']],
    ['Column',               ['attributes' => 'macAddress',                   'label' => 'MAC']],
    ['Column',               ['attributes' => 'fdNetworkInterfaceVlanDN',     'label' => 'VLAN']],
    ['Column',               ['attributes' => 'fdNetworkInterfaceVlanTagged', 'label' => 'VLAN tagged']],
    ['Column',               ['attributes' => 'fdNetworkInterfaceSubnetDN',   'label' => 'Subnet']],
    ['IpColumn',             ['attributes' => 'ipHostNumber',                 'label' => 'IP']],
    ['ActionsColumn',        ['label' => 'Actions']],
  ];

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Interfaces'),
      'plDescription'   => _('Edit the network interfaces of a system'),
      'plIcon'          => 'geticon.php?context=applications&icon=network&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=network&size=16',
      'plObjectType'    => ['server','workstation','terminal','printer','component','phone','mobilePhone'],
      'plPriority'      => 2,

      'plProvidedAcls'  => []
    ];
  }

  function __construct (string $dn, $object, $parent = NULL)
  {
    global $config;

    $this->dn     = $dn;
    if (isset($object->attrs)) {
      $this->attrs = $object->attrs;
      if (isset($object->is_template)) {
        $this->setTemplate($object->is_template);
      }
    }
    $this->parent = $parent;

    parent::__construct(['networkInterface'], []);

    $this->attributesAccess['networkInterfaces'] = new InterfacesAttribute(
      _('Interfaces'), '',
      'networkInterfaces', FALSE
    );
    $this->attributesAccess['networkInterfaces']->setParent($this);

    $this->parent->getBaseObject()->setNetworkAttributesReadOnly(TRUE);

    if ($this->is_template && isset($this->attrs['networkInterfaces'])) {
      unset($this->attrs['networkInterfaces']['count']);
      $this->filter->loadTemplateValues($this->attrs['networkInterfaces']);
    }

    $this->listing->update();

    $entryIterator = $this->listing->getIterator();

    foreach ($entryIterator as $entry) {
      if ($this->is_template) {
        $this->interfaces[$entry->dn] = objects::createTemplate($entry->getTemplatedType());
        $this->filter->resetFromCache($entry->dn, $this->interfaces[$entry->dn]);
      } else {
        $this->interfaces[$entry->dn] = objects::open($entry->dn, $entry->getTemplatedType());
      }
      $this->interfaces[$entry->dn]->getBaseObject()->base = $this->getInterfacesBase();
    }
  }

  private function getInterfacesBase (): string
  {
    if ($this->dn != 'new') {
      return $this->dn;
    } else {
      return 'new,'.$this->parent->getBaseObject()->base;
    }
  }

  protected function setUpListing ()
  {
    /* Set baseMode to FALSE */
    $this->listing  = new managementListing($this, FALSE);
    $this->listing->setBase($this->getInterfacesBase());
  }

  protected function setUpFilter (array $filterElementDefinitions)
  {
    $this->filter   = new interfacesManagementFilter($this, NULL, $filterElementDefinitions);
  }

  /*! \brief    Get used IPs as of current objects states
   */
  public function getUsedIps (): array
  {
    $ips = [];

    foreach ($this->interfaces as $object) {
      if ($object === NULL) {
        continue;
      }
      $ips = array_merge($ips, $object->getBaseObject()->ipHostNumber);
    }

    return $ips;
  }

  function editEntry (array $action)
  {
    global $ui;

    // Do not create a new tabObject while there is already one opened,
    //  the user may have just pressed F5 to reload the page.
    if (is_object($this->tabObject)) {
      return;
    }

    $target = array_pop($action['targets']);

    $entry = $this->listing->getEntry($target);
    if ($entry === NULL) {
      trigger_error('Could not find '.$target.', open canceled');
      return;
    }

    // Get the dn of the object
    $this->currentDn = $target;

    // Open object
    $this->openTabObject($this->interfaces[$this->currentDn]);
    logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->currentDn, 'Edit entry initiated');
    if (isset($action['subaction'])
      && ($this->handleSubAction($action) === FALSE)) {
      trigger_error('Was not able to handle subaction: '.$action['subaction']);
    }
  }

  function saveChanges ()
  {
    if ($this->tabObject instanceOf simpleTabs) {
      $this->tabObject->readPost();
      $this->tabObject->update();
      if ($this->dialogObject instanceOf templateDialog) {
        $this->handleTemplateApply();
      } else {
        $msgs = $this->tabObject->check();
        if (count($msgs)) {
          msg_dialog::displayChecks($msgs);
        } else {
          if ($this->currentDn == 'new') {
            $this->interfaces[$this->tabObject->getBaseObject()->cn] = $this->tabObject;
            $this->filter->updateCache($this->tabObject->getBaseObject()->cn, $this->tabObject);
          } else {
            $this->filter->updateCache($this->currentDn, $this->tabObject);
          }
          logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->currentDns, 'Entry saved in cache');
          $this->closeDialogs();
        }
      }
    }
  }

  function applyChanges ()
  {
    throw new FusionDirectoryException('Not implemented');
  }

  function cancelEdit ()
  {
    if (($this->tabObject instanceOf simpleTabs) && ($this->dialogObject instanceOf templateDialog)) {
      $this->handleTemplateApply(TRUE);
      return;
    }
    $this->filter->resetFromCache($this->currentDn, $this->tabObject);
    $this->closeDialogs();
  }

  function removeRequested (array $action)
  {
    global $ui;
    $disallowed       = [];
    $this->currentDns = [];

    logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $action['targets'], 'Entry deletion requested');

    // Check permissons for each target
    foreach ($action['targets'] as $dn) {
      $entry = $this->listing->getEntry($dn);
      try {
        if ($entry->checkAcl('d')) {
          $this->currentDns[] = $dn;
        } else {
          $disallowed[] = $dn;
        }
      } catch (NonExistingObjectTypeException $e) {
        trigger_error('Unknown object type received :'.$e->getMessage());
      }
    }
    if (count($disallowed)) {
      $error = new FusionDirectoryPermissionError(msgPool::permDelete($disallowed));
      $error->display();
    }

    // We've at least one entry to delete.
    if (count($this->currentDns)) {
      foreach ($this->currentDns as $dn) {
        $this->interfaces[$dn] = NULL;
        $this->filter->updateCache($dn, NULL);
      }
    }
  }

  function openTabObject ($object)
  {
    parent::openTabObject($object);
    $this->tabObject->getBaseObject()->base = $this->getInterfacesBase();
    if ($this->is_template) {
      $this->tabObject->setTemplateMode('');
    }
  }

  public function check (): array
  {
    $errors = [];

    $ips      = [];
    $macvlans = [];
    foreach ($this->interfaces as $object) {
      if ($object === NULL) {
        continue;
      }
      $baseobject = $object->getBaseObject();
      if (class_available('ipamVlan')) {
        $macvlan = $baseobject->macAddress.$baseobject->fdNetworkInterfaceVlanDN;
        if (isset($macvlans[$macvlan])) {
          $errors[] = new SimplePluginCheckError($this, htmlescape(sprintf(_('Interfaces "%s" and "%s" use the same MAC + VLAN combination'), $macvlans[$macvlan], $baseobject->cn)));
        }
        $macvlans[$macvlan] = $baseobject->cn;
      }
      foreach ($baseobject->ipHostNumber as $ip) {
        if (isset($ips[$ip])) {
          $errors[] = new SimplePluginCheckError($this, htmlescape(sprintf(_('Interfaces "%s" and "%s" use the same IP address "%s"'), $ips[$ip], $baseobject->cn, $ip)));
        }
        $ips[$ip] = $baseobject->cn;
      }
    }

    return $errors;
  }

  public function readPost ()
  {
    /* Useful if called for template::update */
    if ($this->attrIsWriteable($this->attributesAccess['networkInterfaces'])) {
      $this->attributesAccess['networkInterfaces']->loadPostValue();
      $this->attributesAccess['networkInterfaces']->applyPostValue();
    }
    parent::readPost();
  }

  public function update (): bool
  {
    parent::update();

    if ($this->renderCache === NULL) {
      if (!$this->dialogOpened()) {
        /* Sync base tab fields with interfaces */
        $entryIterator = $this->listing->getIterator();

        $macAddress   = [];
        $ipHostNumber = [];
        if (!$this->is_template) {
          foreach ($entryIterator as $entry) {
            if (!empty($entry['macAddress'])) {
              array_push($macAddress, ...$entry['macAddress']);
            }
            if (!empty($entry['ipHostNumber'])) {
              array_push($ipHostNumber, ...$entry['ipHostNumber']);
            }
          }
        }
        $this->parent->getBaseObject()->macAddress    = array_values(array_unique($macAddress));
        $this->parent->getBaseObject()->ipHostNumber  = array_values(array_unique($ipHostNumber));
      }
    }

    return TRUE;
  }

  public function remove (bool $fulldelete = FALSE): array
  {
    return [];
  }

  public function save (): array
  {
    global $config;

    if ($this->is_template) {
      return $this->templateLdapSave();
    }

    $errors = [];

    foreach ($this->interfaces as $dn => $object) {
      if ($object === NULL) {
        try {
          $errors = array_merge(
            $errors,
            objects::delete($dn, $this->objectTypes[0])
          );
        } catch (NonExistingLdapNodeException $e) {
          /* No need to delete if it does not exist */
        }
      } else {
        logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Saving subobject');
        /* The move was already done if needed by main tab, so we need to update dn of subobjects */
        $object->dn                     = str_replace($object->getBaseObject()->base, $this->dn, $object->dn);
        $object->getBaseObject()->base = $this->getInterfacesBase();
        /* Save subobject and store errors if any */
        $errors = array_merge($errors, $object->save());
      }
    }

    return $errors;
  }

  protected function templateLdapSave (): array
  {
    global $config;

    /* Check if the entry exists */
    $ldap = $config->get_ldap_link();
    if (!$ldap->dn_exists($this->dn)) {
      return [sprintf(_('The entry %s is not existing'), $this->dn)];
    }

    $attrs = $this->filter->getTemplateAttrs();

    /* Modify */
    $ldap->cd($this->dn);
    $ldap->modify($attrs);

    /* Check for errors */
    if (!$ldap->success()) {
      return [msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class())];
    }
    return [];
  }

  /*! \brief    Prepare active services to be copied.
   */
  public function resetCopyInfos ()
  {
    $this->dn = 'new';
  }

  /*! \brief    Forward plugin acls
   */
  public function set_acl_category (string $category)
  {
    $this->acl_category = $category;
  }

  public function setTemplate (bool $bool)
  {
    $this->is_template = $bool;
  }

  public function getRequiredAttributes (): array
  {
    return [];
  }

  public function setNetworkInterfaces (array $interfaces = NULL)
  {
    if ($interfaces !== NULL) {
      $this->filter->loadTemplateValues($interfaces);
    }

    $this->listing->update();

    $this->listing->setSortColumn(0, FALSE);
    $entryIterator = $this->listing->getIterator();

    /* Build an index of interfaces cn */
    $interfacesCnIndex = array_flip(array_filter(array_map(
      function ($interface) {
        if ($interface !== NULL) {
          return $interface->getBaseObject()->cn;
        } else {
          return NULL;
        }
      },
      $this->interfaces
    )));

    foreach ($entryIterator as $entry) {
      if (isset($interfacesCnIndex[$entry['cn'][0]])) {
        $key = $interfacesCnIndex[$entry['cn'][0]];
        unset($interfacesCnIndex[$entry['cn'][0]]);
      } else {
        if ($this->is_template) {
          $this->interfaces[$entry->dn] = objects::createTemplate($entry->getTemplatedType());
        } else {
          $this->interfaces[$entry->dn] = objects::create($entry->getTemplatedType());
        }
        $key = $entry->dn;
      }
      /* Push values from cache to the object */
      $this->filter->resetFromCache($entry->dn, $this->interfaces[$key]);
      $this->interfaces[$key]->getBaseObject()->base = $this->getInterfacesBase();
    }

    /* Set to NULL the rest of the interfaces array to delete the unused objects upon save */
    foreach ($interfacesCnIndex as $key) {
      $this->interfaces[$key] = NULL;
    }

    /* Fill ip/mac fields of main tab */
    $this->update();
  }

  public function adapt_from_template (array $attrs, array $skip = [])
  {
    $this->attrs = array_merge($this->attrs, $attrs);

    if (isset($this->attrs['networkInterfaces']) && !in_array('networkInterfaces', $skip)) {
      unset($this->attrs['networkInterfaces']['count']);
      $this->setNetworkInterfaces($this->attrs['networkInterfaces']);
    }
  }

  /*!
   * \brief Can we delete the object
   *
   * Only used on main tab
   *
   * \param string $base
   */
  public function acl_is_removeable (string $base = NULL): bool
  {
    return TRUE;
  }

  /*!
   * \brief Sets whether the opened objet has an edit button
   *
   * \param bool $needEditMode
   */
  public function setNeedEditMode (bool $needEditMode)
  {
  }

  /*!
   * \brief Is there a modal dialog opened
   */
  public function is_modal_dialog (): bool
  {
    return $this->dialogOpened();
  }

  /*!
   * \brief Returns TRUE if this attribute should be asked in the creation by template dialog
   */
  public function showInTemplate (string $attr, array $templateAttrs): bool
  {
    return TRUE;
  }

  /*! \brief Check if logged in user have enough right to read this attribute value
   *
   * \param mixed $attr Attribute object or name (in this case it will be fetched from attributesAccess)
   */
  function attrIsReadable ($attr): bool
  {
    if (!is_object($attr)) {
      $attr = $this->attributesAccess[$attr];
    }
    return (strpos($this->aclGetPermissions($attr->getAcl()), 'r') !== FALSE);
  }

  /*! \brief Check if logged in user have enough right to write this attribute value
   *
   * \param mixed $attr Attribute object or name (in this case it will be fetched from attributesAccess)
   */
  function attrIsWriteable ($attr): bool
  {
    if (!is_object($attr)) {
      $attr = $this->attributesAccess[$attr];
    }
    return (strpos($this->aclGetPermissions($attr->getAcl()), 'w') !== FALSE);
  }

  /*!
   * \brief Deserialize values
   */
  public function deserializeValues (array $values, bool $checkAcl = TRUE)
  {
    foreach ($values as $name => $value) {
      if (isset($this->attributesAccess[$name])) {
        if (!$checkAcl || $this->attrIsWriteable($name)) {
          $error = $this->attributesAccess[$name]->deserializeValue($value);
          if (!empty($error)) {
            return $error;
          }
        } else {
          return new FusionDirectoryError(msgPool::permModify($this->dn, $name));
        }
      } else {
        return new FusionDirectoryError(sprintf(_('Unknown field "%s"'), $name));
      }
    }
    return TRUE;
  }

  public function aclHasPermissions (): bool
  {
    return FALSE;
  }

  /*!
   * \brief Get the acl permissions for an attribute or the plugin itself
   */
  public function aclGetPermissions ($attribute = '0', string $base = NULL, bool $skipWrite = FALSE): string
  {
    global $ui;
    if ($attribute !== 'networkInterfaces') {
      return '';
    }
    if (isset($this->parent) && isset($this->parent->ignoreAcls) && $this->parent->ignoreAcls) {
      return 'cdmr'.($skipWrite ? '' : 'w');
    }

    if ($base === NULL) {
      $base = $this->getAclBase();
    }
    $skipWrite  |= $this->read_only;

    $infos  = objects::infos('networkInterface');
    return $ui->get_permissions($base, $infos['aclCategory'].'/'.($this->is_template ? 'template' : $infos['mainTab']));
  }

  /*!
   * \brief Get LDAP base to use for ACL checks
   */
  function getAclBase (bool $callParent = TRUE): string
  {
    global $config;

    if (($this->parent instanceof simpleTabs) && $callParent) {
      return $this->parent->getAclBase();
    }
    if (isset($this->dn) && ($this->dn != 'new')) {
      return $this->dn;
    }
    if (isset($this->base)) {
      return 'new,'.$this->base;
    }

    return $config->current['BASE'];
  }

  /*!
   * \brief Merge in objectClasses needed by this tab
   *
   *  Used by prepare_save and template::apply
   */
  public function mergeObjectClasses (array $oc): array
  {
    return $oc;
  }

  /*!
   * \brief Fill attributes which may be used in hooks
   *
   *  Used by simplePlugin::callHook
   */
  public function fillHookAttrs (array &$addAttrs)
  {
  }

  /*!
   * \brief Test whether a tab is active
   */
  public function isActive (): bool
  {
    return TRUE;
  }

  /*!
   * \brief Test whether a tab can be deactivated
   */
  public function isActivatable (): bool
  {
    return FALSE;
  }
}
