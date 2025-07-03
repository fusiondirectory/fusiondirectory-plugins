<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2025  FusionDirectory

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

/*! \brief This class extends SetAttribute to ensure mail validation is performed
 * when adding emails to the set. (Prior to the saving mechanism).
 */
class SupannMailSetAttribute extends SetAttribute
{
  /*! \brief The constructor of SupannMailSetAttribute
   *
   *  \param MailAttribute $attribute The mail attribute to make multi-valued with validation
   *  \param array $values The default values
   *  \param boolean $valueUnicity Should the value unicity be checked
   */
  function __construct (MailAttribute $attribute, array $values = [], bool $valueUnicity = TRUE)
  {
    parent::__construct($attribute, $values, $valueUnicity);
  }

  /*! \brief Override the addPostValue method to validate email before adding
   *
   * \param mixed $value The value to add to the set
   * \return boolean TRUE if the value was added, FALSE otherwise
   */
  function addPostValue ($value)
  {
    // First check if the value is empty or already exists in the set
    if ($value === '') {
      return FALSE;
    }

    // Trim the value to remove leading/trailing whitespaces
    $value = trim($value);

    if ($this->valueUnicity && in_array($value, $this->postValue, TRUE)) {
      $error = new SimplePluginCheckError(
        $this,
        sprintf(_('Email address "%s" already exists in the list'), $value)
      );
      $error->display();
      return FALSE;
    }

    // Validate email format before adding
    if (!tests::is_email($value)) {
      $error = new SimplePluginCheckError(
        $this,
        SimplePluginCheckError::invalidValue(sprintf(_('"%s" is not a valid email address'), $value))
      );
      $error->display();
      return FALSE;
    }

    // Value passed validation, add it to the set
    $this->postValue[] = $value;
    return TRUE;
  }

  /*! \brief Override handleAddAndEditValue to ensure validation is performed when editing
   */
  protected function handleAddAndEditValue ()
  {
    $id = $this->getHtmlId();
    if ($this->attribute === FALSE) {
      return;
    }
    if (isset($_POST["add$id"])) {
      $this->attribute->loadPostValue();
      $this->attribute->applyPostValue();

      // Get value and perform validation
      $value = $this->attribute->getValue();

      // Use the parent check first
      if ($error = $this->attribute->check()) {
        if (is_string($error)) {
          $error = new SimplePluginCheckError($this, $error);
        }
        $error->display();
      } else {
        // If parent check passed, add the value (which will do our email validation)
        $this->addPostValue($value);
      }
    } elseif ($this->editingValue === FALSE) {
      $this->attribute->loadPostValue();
      $this->attribute->applyPostValue();
      $this->editingValue = $this->attribute->getValue();
    }
  }

  /*! \brief Validate all values in the set
   *
   * \return array of SimplePluginCheckError objects or empty array if all values are valid
   */
  function check ()
  {
    $errors = parent::check();

    // Check each email in the set for validity
    foreach ($this->getValue() as $email) {
      if (!tests::is_email($email)) {
        $errors[] = new SimplePluginCheckError(
          $this,
          SimplePluginCheckError::invalidValue(sprintf(_('"%s" is not a valid email address'), $email))
        );
      }
    }

    return $errors;
  }
}
