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
  public $ignore_account  = TRUE;
  public $dn;
  public $is_template     = FALSE;
  public $parent          = NULL;
  public $acl_category    = '';

  protected $current      = '';
  protected $backup       = NULL;

  var $read_only      = FALSE;
  var $acl;
  var $cn;

  public static $columns = [
    ['LinkColumn',           ['attributes' => 'nameAttr',                   'label' => 'Name']],
    ['Column',               ['attributes' => 'macAddress',                 'label' => 'Mac']],
    ['Column',               ['attributes' => 'fdNetworkInterfaceVlanId',   'label' => 'VLAN id']],
    ['Column',               ['attributes' => 'fdNetworkInterfaceVlanTag',  'label' => 'VLAN tag']],
    ['Column',               ['attributes' => 'fdNetworkInterfaceSubnetDN', 'label' => 'Subnet']],
    ['IpColumn',             ['attributes' => 'ipHostNumber',               'label' => 'IP']],
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
    }
    $this->parent = $parent;

    parent::__construct(['networkInterface'], []);

    $this->parent->getBaseObject()->setNetworkAttributesReadOnly(TRUE);
  }

  protected function setUpListing ()
  {
    /* Set baseMode to FALSE */
    $this->listing  = new managementListing($this, FALSE);
    $this->listing->setBase($this->dn);
  }

  function openTabObject ($object)
  {
    parent::openTabObject($object);
    $this->tabObject->getBaseObject()->base = $this->dn;
  }

  /*! \brief    No checks here.
   */
  public function check (): array
  {
    return [];
  }

  /*! \brief    Keep posted form values in opened dialogs
   */
  function save_object ()
  {
    // save_object of the dialog is called in management::execute

    /* Sync base tab fields with interfaces */
    $entryIterator = $this->listing->getIterator();

    $macAddress   = [];
    $ipHostNumber = [];
    foreach ($entryIterator as $entry) {
      if (!empty($entry['macAddress'])) {
        array_push($macAddress, ...$entry['macAddress']);
      }
      if (!empty($entry['ipHostNumber'])) {
        array_push($ipHostNumber, ...$entry['ipHostNumber']);
      }
    }
    $this->parent->getBaseObject()->macAddress    = array_values(array_unique($macAddress));
    $this->parent->getBaseObject()->ipHostNumber  = array_values(array_unique($ipHostNumber));
  }

  public function remove (bool $fulldelete = FALSE): array
  {
    return [];
  }

  public function save (): array
  {
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
  }

  public function getRequiredAttributes (): array
  {
    return [];
  }

  public function adapt_from_template (array $attrs, array $skip = [])
  {
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
    return FALSE;
  }

  /*!
   * \brief Deserialize values
   */
  public function deserializeValues (array $values, bool $checkAcl = TRUE)
  {
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
    return '';
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
}
