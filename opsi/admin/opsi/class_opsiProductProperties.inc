<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013 FusionDirectory

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

class opsiProductProperties extends simplePlugin
{
  var $objectclasses   = array('opsiProductProperties');

  public static function plInfo()
  {
    return array(
      'plShortName'   => _('Product properties'),
      'plDescription' => _('Product properties'),
      'plSelfModify'  => FALSE,
      'plCategory'    => array('opsiProfile'),
      'plObjectType'  => array('opsiProfile'),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Name'),
        'attrs' => array(
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('Repositories this server hosts'),
              'FAIrepository',
              array(
                new SelectAttribute(
                  '', '',
                  'productId', TRUE,
                  array('product1','product2'), '',
                  array('Product 1', 'Product 2')
                ),
                new SelectAttribute(
                  '', '',
                  'propertiesId', TRUE,
                  array('property1','property2'), '',
                  array('Property 1', 'Property 2')
                ),
                new StringAttribute(
                  '', '',
                  'propertiesValue', TRUE
                ),
              ),
              '|'
            ),
            FALSE,  // no order
            array('product1|property1|true', 'product2|property1|false'),
            TRUE    // edit button
          )
        )
      )
    );
  }
}
?>
