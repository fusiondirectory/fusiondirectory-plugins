<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2013-2019  FusionDirectory

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

class WebServiceError extends FusionDirectoryException
{
  public function toArray ()
  {
    return [
      'message' => $this->getMessage(),
      'line'    => $this->getLine(),
      'file'    => $this->getFile(),
    ];
  }
}

class WebServiceErrors extends FusionDirectoryException
{
  protected $errors;

  public function __construct (array $errors)
  {
    parent::__construct();
    $this->errors = $errors;
    foreach ($this->errors as &$error) {
      if (is_string($error)) {
        $error = new WebServiceError($error);
      }
    }
    unset($error);
  }

  public function toJson ()
  {
    return json_encode(
      array_map(
        function ($error)
        {
          return $error->toArray();
        },
        $this->errors
      )
    );
  }

  public function toStringArray ()
  {
    return array_map(
      function ($e) {
        return $e->getMessage();
      },
      $this->errors
    );
  }
}

/*!
 * \brief This class is the webservice of FusionDirectory
 *
 * It may be served through jsonRPCServer::handle
 * */
class fdRPCService
{
  function __construct ()
  {
  }

  public static function authenticateHeader ($message = 'Authentication required')
  {
    header('WWW-Authenticate: Basic realm="FusionDirectory"');
    header('HTTP/1.0 401 Unauthorized');
    echo "$message\n";
    exit;
  }

  public static function initiateRPCSession ($id = NULL, $ldap = NULL, $user = NULL, $pwd = NULL, bool $userLogin = TRUE)
  {
    global $config, $class_mapping, $BASE_DIR, $ui, $ssl;

    session::start($id);

    /* Reset errors */
    reset_errors();
    /* Check if CONFIG_FILE is accessible */
    if (!is_readable(CONFIG_DIR."/".CONFIG_FILE)) {
      throw new FusionDirectoryException(sprintf(_("FusionDirectory configuration %s/%s is not readable. Aborted."), CONFIG_DIR, CONFIG_FILE));
    }

    /* Initially load all classes */
    $class_list = get_declared_classes();
    foreach ($class_mapping as $class => $path) {
      if (!in_array($class, $class_list)) {
        if (is_readable("$BASE_DIR/$path")) {
          require_once("$BASE_DIR/$path");
        } else {
          throw new FusionDirectoryException(sprintf(_("Cannot locate file '%s' - please run '%s' to fix this"),
                "$BASE_DIR/$path", "<b>fusiondirectory-setup</b>"));
        }
      }
    }
    /* Parse configuration file */
    if (session::global_is_set('LOGIN') && session::global_is_set('config') && session::global_is_set('plist')) {
      $config = session::global_get('config');
      $plist  = session::global_get('plist');
      $ui     = session::global_get('ui');
      Language::init();
    } else {
      $config = new config(CONFIG_DIR."/".CONFIG_FILE, $BASE_DIR);
      if ($ldap === NULL) {
        $config->set_current(key($config->data['LOCATIONS']));
      } elseif (isset($config->data['LOCATIONS'][$ldap])) {
        $config->set_current($ldap);
      } else {
        static::authenticateHeader('Invalid LDAP specified');
      }
      session::global_set('DEBUGLEVEL', 0);

      if (($ssl != '') &&
            (($config->get_cfg_value('webserviceForceSSL', 'TRUE') == 'TRUE') ||
             ($config->get_cfg_value('forcessl') == 'TRUE'))) {
        throw new FusionDirectoryException("HTTP connexions are not allowed, please use HTTPS: $ssl\n");
      }

      if ($userLogin) {
        if (!isset($_SERVER['PHP_AUTH_USER']) && ($user === NULL)) {
          static::authenticateHeader();
        }
        if ($user !== NULL) {
          $ui = ldap_login_user($user, $pwd);
          if (!$ui) {
            static::authenticateHeader('Invalid user or pwd '.$user.'/'.$pwd);
          }
        } else {
          $ui = ldap_login_user($_SERVER['PHP_AUTH_USER'], $_SERVER['PHP_AUTH_PW']);
          if (!$ui) {
            static::authenticateHeader('Invalid user or pwd '.$_SERVER['PHP_AUTH_USER'].'/'.$_SERVER['PHP_AUTH_PW']);
          }
        }
        session::global_set('LOGIN', TRUE);
        $plist = new pluglist();
        $config->loadPlist($plist);
        $config->get_departments();
        $config->make_idepartments();
        session::global_set('ui',     $ui);
        session::global_set('plist',  $plist);
        session::global_set('config', $config);
      }
    }
  }

  protected function checkAccess ($type, $tabs = NULL, $dn = NULL)
  {
    global $ui;
    $infos = objects::infos($type);
    $plist = session::global_get('plist');
    if (($dn !== NULL) && ($ui->dn == $dn)) {
      $self = ':self';
    } else {
      $self = '';
    }
    if (!$plist->check_access(['ACL' => $infos['aclCategory'].$self])) {
      throw new WebServiceError(sprintf('Unsufficient rights for accessing type "%s%s"', $type, $self), 403);
    }
    if ($tabs !== NULL) {
      if (!is_array($tabs)) {
        $tabs = [$tabs];
      }
      foreach ($tabs as $tab) {
        $pInfos = pluglist::pluginInfos($tab);
        if ($pInfos === NULL) {
          throw new WebServiceError(sprintf('Tab "%s" of type "%s" does not exists', $tab, $type), 404);
        }
        if (!isset($pInfos['plCategory'])) {
          throw new WebServiceError(sprintf('Tab "%s" of type "%s" has no ACL category', $tab, $type), 500);
        }
        if (!$plist->check_access(['ACL' => join($self.',', $pInfos['plCategory']).$self])) {
          throw new WebServiceError(sprintf('Unsufficient rights for accessing tab "%s" of type "%s%s"', $tab, $type, $self), 403);
        }
      }
    }
  }

  /*!
   * \brief Log out
   */
  protected function _logout ()
  {
    session::destroy();
    return TRUE;
  }

  /*!
   * \brief Get list of object of objectType $type in $ou
   */
  protected function _ls ($type, $attrs = NULL, $ou = NULL, $filter = '', $scope = 'subtree', $templateSearch = FALSE)
  {
    global $config, $ui;
    $this->checkAccess($type);
    if (!in_array($scope, ['base','one','subtree'])) {
      throw new WebServiceError(sprintf('Invalid scope "%s"', $scope));
    }
    return objects::ls($type, $attrs, $ou, $filter, TRUE, $scope, $templateSearch);
  }

  /*!
   * \brief Get count of objects of objectType $type in $ou
   */
  protected function _count ($type, $ou = NULL, $filter = '')
  {
    global $ui;
    $this->checkAccess($type);
    $infos  = objects::infos($type);
    $acl    = $infos['aclCategory'].'/'.$infos['mainTab'];
    if ((strpos($ui->get_permissions($ou, $acl), 'r') !== FALSE) && ($filter == '')) {
      return objects::count($type, $ou, $filter, TRUE);
    } else {
      return count(objects::ls($type, NULL, $ou, $filter, TRUE));
    }
  }

  /*!
   * \brief Get information about objectType $type
   */
  protected function _infos ($type)
  {
    $this->checkAccess($type);

    global $config;
    $infos = objects::infos($type);
    unset($infos['tabClass']);
    $infos['tabs'] = $config->data['TABS'][$infos['tabGroup']];
    unset($infos['tabGroup']);
    return $infos;
  }

  /*!
   * \brief List existing object types
   */
  protected function _listTypes ()
  {
    $types  = objects::types();

    $result = [];
    foreach ($types as $type) {
      $infos          = objects::infos($type);
      $result[$type]  = $infos['name'];
    }
    return $result;
  }

  /*!
   * \brief List tabs on an object
   */
  protected function _listTabs ($type, $dn = NULL)
  {
    $this->checkAccess($type, NULL, $dn);

    if ($dn === NULL) {
      $tabobject = objects::create($type);
    } else {
      $tabobject = objects::open($dn, $type);
    }

    $tabs = [];
    foreach ($tabobject->by_object as $tab => $obj) {
      $tabs[$tab] = [
        'name'    => $tabobject->by_name[$tab],
        'active'  => ($obj->is_account || $obj->ignore_account)
      ];
    }
    return $tabs;
  }

  /*!
   * \brief Deactivate a tab of an object
   */
  protected function _removetab (string $type, string $dn, string $tab): string
  {
    $this->checkAccess($type, $tab, $dn);
    $tabobject = objects::open($dn, $type);
    $tabobject->current = $tab;
    if (!is_subclass_of($tabobject->by_object[$tab], 'simplePlugin')) {
      throw new WebServiceError('Tab '.$tab.' is not based on simplePlugin, can’t remove it', 501);
    } elseif (!$tabobject->by_object[$tab]->displayHeader) {
      throw new WebServiceError('Tab '.$tab.' cannot be deactivated, can’t remove it');
    } elseif (!$tabobject->by_object[$tab]->is_account) {
      throw new WebServiceError('Tab '.$tab.' is not activated on "'.$dn.'", can’t remove it');
    } elseif (!$tabobject->by_object[$tab]->acl_is_removeable()) {
      throw new WebServiceError('You don\'t have sufficient rights to disable tab "'.$tab.'"', 403);
    } else {
      list($disabled, $buttonText, $text) = $tabobject->by_object[$tab]->getDisplayHeaderInfos();
      if ($disabled) {
        throw new WebServiceError($text);
      }
    }
    $_POST = [$tab.'_modify_state' => 1];
    $tabobject->save_object();
    $errors = $tabobject->save();
    if (!empty($errors)) {
      throw new WebServiceErrors($errors);
    }
    return $tabobject->dn;
  }

  /*!
   * \brief Get all form fields from an object (or object type)
   */
  protected function _formfields ($type, $dn = NULL, $tab = NULL)
  {
    $this->checkAccess($type, $tab, $dn);

    if ($dn === NULL) {
      $tabobject = objects::create($type);
    } else {
      $tabobject = objects::open($dn, $type);
    }
    if ($tab === NULL) {
      $object = $tabobject->getBaseObject();
    } else {
      $object = $tabobject->by_object[$tab];
    }
    if (is_subclass_of($object, 'simplePlugin')) {
      $fields = $object->attributesInfo;
      foreach ($fields as &$section) {
        $attributes = [];
        foreach ($section['attrs'] as $attr) {
          if ($object->acl_is_readable($attr->getAcl())) {
            $attr->serializeAttribute($attributes, TRUE);
          }
        }
        $section['attrs']       = $attributes;
        $section['attrs_order'] = array_keys($attributes);
      }
      unset($section);
    } else {
      /* fallback for old plugins */
      $fields = ['main' => ['attrs' => [], 'name' => _('Plugin')]];
      foreach ($object->attributes as $attr) {
        if ($object->acl_is_readable($attr.'Acl')) {
          $fields['main']['attrs'][$attr] = [
            'value'       => $object->$attr,
            'required'    => FALSE,
            'disabled'    => FALSE,
            'label'       => $attr,
            'type'        => 'OldPluginAttribute',
            'description' => '',
          ];
        }
      }
      $fields['main']['attrs_order'] = array_keys($fields['main']['attrs']);
    }
    return ['sections' => $fields, 'sections_order' => array_keys($fields)];
  }

  /*!
   * \brief Update values of an object's attributes using POST as if the webpage was sent
   */
  protected function _formpost ($type, $dn, $tab, $values)
  {
    $this->checkAccess($type, $tab, $dn);

    if ($dn === NULL) {
      $tabobject = objects::create($type);
    } else {
      $tabobject = objects::open($dn, $type);
    }
    if ($tab === NULL) {
      $tab = $tabobject->current;
    } else {
      $tabobject->current = $tab;
    }
    $_POST                  = $values;
    $_POST[$tab.'_posted']  = TRUE;
    if (is_subclass_of($tabobject->by_object[$tab], 'simplePlugin') &&
        $tabobject->by_object[$tab]->displayHeader &&
        !$tabobject->by_object[$tab]->is_account
      ) {
      $_POST[$tab.'_modify_state'] = 1;
    }
    $tabobject->save_object();
    $errors = $tabobject->save();
    if (!empty($errors)) {
      throw new WebServiceErrors($errors);
    }
    return $tabobject->dn;
  }

  /*!
   * \brief Get all internal fields from an object (or object type)
   */
  protected function _getfields ($type, $dn = NULL, $tab = NULL)
  {
    $this->checkAccess($type, $tab, $dn);

    if ($dn === NULL) {
      $tabobject = objects::create($type);
    } else {
      $tabobject = objects::open($dn, $type);
    }
    if ($tab === NULL) {
      $object = $tabobject->getBaseObject();
    } else {
      $object = $tabobject->by_object[$tab];
    }
    if (is_subclass_of($object, 'simplePlugin')) {
      $fields = $object->attributesInfo;
      foreach ($fields as &$section) {
        $attributes = [];
        foreach ($section['attrs'] as $attr) {
          if ($object->acl_is_readable($attr->getAcl())) {
            $attr->serializeAttribute($attributes, FALSE);
          }
        }
        $section['attrs']       = $attributes;
        $section['attrs_order'] = array_keys($attributes);
      }
      unset($section);
    } else {
      /* fallback for old plugins */
      $fields = ['main' => ['attrs' => [], 'name' => _('Plugin')]];
      foreach ($object->attributes as $attr) {
        if ($object->acl_is_readable($attr.'Acl')) {
          $fields['main']['attrs'][$attr] = $object->$attr;
        }
      }
      $fields['main']['attrs_order'] = array_keys($fields['main']['attrs']);
    }
    return ['sections' => $fields, 'sections_order' => array_keys($fields)];
  }

  /*!
   * \brief Set internal values of an object's attributes and save it
   */
  protected function _setfields (string $type, $dn, $values): string
  {
    $this->checkAccess($type, array_keys($values), $dn);
    if ($dn === NULL) {
      $tabobject = objects::create($type);
    } else {
      $tabobject = objects::open($dn, $type);
    }
    foreach ($values as $tab => $tabvalues) {
      if (!isset($tabobject->by_object[$tab])) {
        throw new WebServiceError('This tab does not exists: "'.$tab.'"', 404);
      }
      if (is_subclass_of($tabobject->by_object[$tab], 'simplePlugin') &&
          $tabobject->by_object[$tab]->displayHeader &&
          !$tabobject->by_object[$tab]->is_account
        ) {
        list($disabled, $buttonText, $text) = $tabobject->by_object[$tab]->getDisplayHeaderInfos();
        if ($disabled) {
          throw new WebServiceError($text);
        }
        if ($tabobject->by_object[$tab]->acl_is_createable()) {
          $tabobject->by_object[$tab]->is_account = TRUE;
        } else {
          throw new WebServiceError('You don\'t have sufficient rights to enable tab "'.$tab.'"', 403);
        }
      }
      $error = $tabobject->by_object[$tab]->deserializeValues($tabvalues);
      if ($error !== TRUE) {
        throw new WebServiceError($error);
      }
      $tabobject->current = $tab;
      $tabobject->save_object(); /* Should not do much as POST is empty, but in some cases is needed */
    }
    $errors = $tabobject->save();
    if (!empty($errors)) {
      throw new WebServiceErrors($errors);
    }
    return $tabobject->dn;
  }

  /*!
   * \brief Add values to an object's attributes and save it
   */
  protected function _addvalues (string $type, $dn, $values): string
  {
    return $this->adddelvalues($type, $dn, $values, TRUE);
  }

  /*!
   * \brief Delete values from an object's attributes and save it
   */
  protected function _delvalues (string $type, $dn, $values): string
  {
    return $this->adddelvalues($type, $dn, $values, FALSE);
  }

  private function adddelvalues (string $type, $dn, $values, bool $add): string
  {
    $this->checkAccess($type, array_keys($values), $dn);
    if ($dn === NULL) {
      throw new WebServiceError('No DN provided');
    } else {
      $tabobject = objects::open($dn, $type);
    }
    foreach ($values as $tab => $tabvalues) {
      if (!isset($tabobject->by_object[$tab])) {
        throw new WebServiceError('This tab does not exists: "'.$tab.'"', 404);
      }
      if (is_subclass_of($tabobject->by_object[$tab], 'simplePlugin') &&
          $tabobject->by_object[$tab]->displayHeader &&
          !$tabobject->by_object[$tab]->is_account
        ) {
        if ($tabobject->by_object[$tab]->acl_is_createable()) {
          $tabobject->by_object[$tab]->is_account = TRUE;
        } else {
          throw new WebServiceError('You don\'t have sufficient rights to enable tab "'.$tab.'"', 403);
        }
      }
      foreach ($tabvalues as $name => $newvalues) {
        if (isset($tabobject->by_object[$tab]->attributesAccess[$name])) {
          if ($tabobject->by_object[$tab]->attrIsWriteable($name)) {
            $attrvalues = $tabobject->by_object[$tab]->attributesAccess[$name]->getValue();
            if (!is_array($attrvalues)) {
              throw new WebServiceError(sprintf(_('Field "%s" is not multi-valuated'), $name));
            }
            if (!is_array($newvalues)) {
              $newvalues = [$newvalues];
            }
            if ($add) {
              $attrvalues = array_merge($attrvalues, $newvalues);
            } else {
              $attrvalues = array_remove_entries($newvalues, $attrvalues);
            }
            $tabobject->by_object[$tab]->attributesAccess[$name]->setValue($attrvalues);
          } else {
            throw new WebServiceError(msgPool::permModify($dn, $name), 403);
          }
        } else {
          throw new WebServiceError(sprintf(_('Unknown field "%s"'), $name), 404);
        }
      }
      $tabobject->current = $tab;
      $tabobject->save_object(); /* Should not do much as POST is empty, but in some cases is needed */
    }
    $errors = $tabobject->save();
    if (!empty($errors)) {
      throw new WebServiceErrors($errors);
    }
    return $tabobject->dn;
  }

  /*!
   * \brief Get all internal fields from a template
   */
  protected function _gettemplate ($type, $dn)
  {
    $this->checkAccess($type, NULL, $dn);

    $template = new template($type, $dn);
    return $template->serialize();
  }

  /*!
   * \brief
   */
  protected function _usetemplate (string $type, string $dn, $values): string
  {
    $this->checkAccess($type, NULL, $dn);

    $template = new template($type, $dn);
    $error    = $template->deserialize($values);
    if ($error !== TRUE) {
      throw new WebServiceError($error);
    }
    $tabobject = $template->apply();
    $errors = $tabobject->save();
    if (!empty($errors)) {
      throw new WebServiceErrors($errors);
    }
    return $tabobject->dn;
  }

  /*!
   * \brief Delete an object
   */
  protected function _delete (string $type, string $dn)
  {
    global $ui;
    $infos = objects::infos($type);
    // Check permissions, are we allowed to remove this object?
    $acl = $ui->get_permissions($dn, $infos['aclCategory'].'/'.$infos['mainTab']);
    if (strpos($acl, 'd') === FALSE) {
      throw new WebServiceError(msgPool::permDelete($dn), 403);
    }

    if ($user = get_lock($dn)) {
      throw new WebServiceError(sprintf(_('Cannot delete %s. It has been locked by %s.'), $dn, $user));
    }
    add_lock($dn, $ui->dn);

    // Delete the object
    $tabobject = objects::open($dn, $type);
    $tabobject->delete();

    // Remove the lock for the current object.
    del_lock($dn);
  }

  /*!
   * \brief Lock or unlock a user
   */
  protected function _lockUser ($dns, $type = 'toggle')
  {
    global $config, $ui;

    // Filter out entries we are not allowed to modify
    $disallowed = [];

    if (!is_array($dns)) {
      $dns = [$dns];
    }

    foreach ($dns as $dn) {
      if (strpos($ui->get_permissions($dn, 'user/user', 'userLock'), 'w') === FALSE) {
        $disallowed[] = $dn;
      }
    }
    if (count($disallowed)) {
      throw new WebServiceError(msgPool::permModify($disallowed), 403);
    }

    // Try to lock/unlock the entries.
    $ldap   = $config->get_ldap_link();
    $errors = [];
    foreach ($dns as $dn) {
      $ldap->cat($dn, ['userPassword']);
      if ($ldap->count() == 1) {
        // We can't lock empty passwords.
        $val = $ldap->fetch();
        if (!isset($val['userPassword'])) {
          $errors[] = sprintf(_('Failed to get password method for account "%s". It has not been locked!'), $dn);
          continue;
        }
        // Detect the password method and try to lock/unlock.
        $method = passwordMethod::get_method($val['userPassword'][0], $dn);
        if ($method instanceOf passwordMethod) {
          $success = TRUE;
          if ($type == 'toggle') {
            if ($method->is_locked($dn)) {
              $success = $method->unlock_account($dn);
            } else {
              $success = $method->lock_account($dn);
            }
          } elseif ($type == 'lock' && !$method->is_locked($dn)) {
            $success = $method->lock_account($dn);
          } elseif ($type == 'unlock' && $method->is_locked($dn)) {
            $success = $method->unlock_account($dn);
          }

          // Check if everything went fine.
          if (!$success) {
            $hn = $method->get_hash_name();
            if (is_array($hn)) {
              $hn = $hn[0];
            }
            $errors[] = sprintf(_('Password method "%s" failed locking. Account "%s" has not been locked!'), $hn, $dn);
          }
        } else {
          // Can't lock unknown methods.
          $errors[] = sprintf(_('Failed to get password method for account "%s". It has not been locked!'), $dn);
        }
      } else {
        $errors[] = sprintf(_('Could not find account "%s" in LDAP. It has not been locked!'), $dn);
      }
    }
    if (!empty($errors)) {
      throw new WebServiceErrors($errors);
    }
  }

  /*!
   * \brief Test if a user is locked
   */
  protected function _isUserLocked ($dns)
  {
    global $config, $ui;

    if (!is_array($dns)) {
      $dns = [$dns];
    }

    foreach ($dns as $dn) {
      if (strpos($ui->get_permissions($dn, 'user/user', 'userLock'), 'r') === FALSE) {
        $disallowed[] = $dn;
      }
    }
    if (count($disallowed)) {
      throw new WebServiceError(msgPool::permView($disallowed), 403);
    }

    // Try to lock/unlock the entries.
    $ldap   = $config->get_ldap_link();
    $result = [];
    $errors = [];
    foreach ($dns as $dn) {
      $ldap->cat($dn, ['userPassword']);
      if ($ldap->count() == 1) {
        // We can't lock empty passwords.
        $val = $ldap->fetch();
        if (!isset($val['userPassword'])) {
          $errors[] = sprintf(_('Failed to get password method for account "%s"'), $dn);
          continue;
        }
        // Detect the password method and try to lock/unlock.
        $method = passwordMethod::get_method($val['userPassword'][0], $dn);
        if ($method instanceOf passwordMethod) {
          $result[$dn] = $method->is_locked($dn);
        } else {
          // Can't lock unknown methods.
          $errors[] = sprintf(_('Failed to get password method for account "%s"'), $dn);
        }
      } else {
        $errors[] = sprintf(_('Could not find account "%s" in LDAP'), $dn);
      }
    }
    if (!empty($errors)) {
      throw new WebServiceErrors($errors);
    }
    return $result;
  }

  /*!
   * \brief Generate recovery token for a user
   */
  protected function _recoveryGenToken ($email)
  {
    global $ui;

    $pwRecovery = new passwordRecovery(FALSE);
    $dn = $pwRecovery->step2($email);
    if ($dn !== FALSE) {
      /* No errors */
      if (strpos($ui->get_permissions($dn, 'user/user', 'userPassword'), 'w') === FALSE) {
        throw new WebServiceError(msgPool::permModify($dn), 403);
      }
      $token = $pwRecovery->generateAndStoreToken();
      if ($token !== FALSE) {
        return ['token' => $token, 'login' => $pwRecovery->getLogin()];
      }
    }
    throw new WebServiceErrors($pwRecovery->getErrorMessages());
  }

  /*!
   * \brief Change a user password using a recovery token
   */
  protected function _recoveryConfirmPasswordChange ($login, $password1, $password2, $token)
  {
    $pwRecovery = new passwordRecovery(FALSE);
    $pwRecovery->setLogin($login);
    if ($pwRecovery->checkToken($token)) {
      $success = $pwRecovery->changeUserPassword($password1, $password2);
      if ($success) {
        return TRUE;
      }
    } else {
      throw new WebServiceError(_('This token is invalid'));
    }
    throw new WebServiceErrors($pwRecovery->getErrorMessages());
  }

  /*!
   * \brief Get the session ID
   */
  protected function _getId ()
  {
    return session_id();
  }

  /*!
   * \brief Get the LDAP base
   */
  protected function _getBase ()
  {
    global $config;
    return $config->current['BASE'];
  }
}
