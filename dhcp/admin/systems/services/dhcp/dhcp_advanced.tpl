{* GOsa dhcp sharedNetwork - smarty template *}

<p class='seperator'></p>
<br>

{if $show_advanced}

<input type='submit' name='hide_advanced' value='{t}Hide advanced settings{/t}'>

<table width="100%">
 <tr>

  <td width="50%">
   <br>
   <b>{t}DHCP statements{/t}</b>
   <br>
{render acl=$acl}
   <select name='dhcpstatements' style="width:100%;" size="14">
    {html_options options=$dhcpstatements}
   </select>
{/render}
   <br>
{render acl=$acl}
   <input type='text' name='addstatement' size='25' maxlength='250'>&nbsp;
{/render}
{render acl=$acl}
   <input type='submit' name='add_statement' value='{msgPool type=addButton}'>&nbsp;
{/render}
{render acl=$acl}
   <input type='submit' name='delete_statement' value='{msgPool type=delButton}'> 
{/render}
  </td>

  <td>
   <br>
   <b>{t}DHCP options{/t}</b>
   <br>
{render acl=$acl}
   <select name='dhcpoptions' style="width:100%;" size="14">
    {html_options options=$dhcpoptions}
   </select>
{/render}
   <br>
{render acl=$acl}
   <input type='text' name='addoption' size='25' maxlength='250'>&nbsp;
{/render}
{render acl=$acl}
   <input type='submit' name='add_option' value='{msgPool type=addButton}'>&nbsp;
{/render}
{render acl=$acl}
   <input type='submit' name='delete_option' value='{msgPool type=delButton}'> 
{/render}
  </td>
 </tr>
</table>

{else}

<input type='submit' name='show_advanced' value='{t}Show advanced settings{/t}'>

{/if}
<p class='seperator'>&nbsp;</p>
