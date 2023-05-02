<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2017-2019 FusionDirectory

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

class ServiceListingEntry extends ListingEntry
{
  public function __construct (managementListing $listing, string $name, array $attrs, int $row = NULL)
  {
    parent::__construct($listing, 'service_'.$name, $name, $attrs, $row);
    $this->aclBase = $listing->parent->dn;
  }

  public function checkAcl (string $acls): bool
  {
    global $ui;

    $infos  = objects::infos('server');
    $rights = $ui->get_permissions($this->dn, $infos['aclCategory'].'/'.$this->getServiceName());
    foreach (str_split($acls) as $acl) {
      if (strpos($rights, $acl) === FALSE) {
        return FALSE;
      }
    }

    return TRUE;
  }

  public function getServiceName (): string
  {
    return preg_replace('/^service_/', '', $this->type);
  }

  public function snapshotCreationAllowed (): bool
  {
    return FALSE;
  }

  public function snapshotRestoreAllowed (): bool
  {
    return FALSE;
  }
}
