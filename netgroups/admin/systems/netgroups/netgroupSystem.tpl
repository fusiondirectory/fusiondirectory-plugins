{if $multiple_support}
<input type="hidden" value="1" name="nisnetgroup_mulitple_edit">
{/if}


<table summary="" style="width:100%;">
 <tr>
  <td style="width:1%; vertical-align:top;">

  </td>

  <td style="vertical-align:top;">

   <table summary="" style="width:100%">
    <tr>
     <td style="vertical-align:top; width:50%">
      <b><LABEL for="members">{t}Member of the following NIS Netgroups{/t}</LABEL></b>
      <br>
{render acl=$memberCnACL}

	{if $multiple_support}
	  <select style="width:100%; height:380px;" id="members" name="members[]" size=15 multiple>
		{foreach from=$memberCn_All item=name key=key}
			<option value="{$key}">{$name}&nbsp;({t}In all NIS Netgroups{/t})</option>
		{/foreach}
		{foreach from=$memberCn_Some item=name key=key}
        <option value="{$key}" style='color: #888888; background: #DDDDDD;background-color: #DDDDDD;'>{$name}&nbsp;({t}Not in all NIS Netgroups{/t})</option>
		{/foreach}
	  </select>
	{else}
      <select style="width:100%; height:380px;" id="members" name="members[]" size=15 multiple>
       {html_options options=$members}
		<option disabled>&nbsp;</option>
      </select>
	{/if}
{/render}
      <br>
{render acl=$memberCnACL}
      <input type=submit name="edit_membership" value="{msgPool type=addButton}">
{/render}
      &nbsp;
{render acl=$memberCnACL}
      <input type=submit name="del_users" value="{msgPool type=delButton}">
{/render}
     </td>
    </tr> 
   </table>
  </td>
  <td style="border-left:1px solid #A0A0A0">
   &nbsp;
  </td>
  <td style="width:30%; vertical-align:top;">

  </td>
 </tr>
</table>

<input type="hidden" name="nisnetgroupedit" value="1">

