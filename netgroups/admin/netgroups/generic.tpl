{if $multiple_support}
<input type="hidden" value="1" name="nisnetgroup_mulitple_edit">
{/if}


<table summary="" style="width:100%;">
 <tr>
  <td style="width:50%; vertical-align:top;">
   <table summary="" style="width:100%">
    <tr>
     <td><LABEL for="cn">{t}NIS Netgroup name{/t}</LABEL>{$must}</td>
     <td>
{if $multiple_support}
  <input type='text' id="dummy1" name="dummy1" size=25 maxlength=60 value="{t}Multiple edit{/t}" disabled>
{else}
{render acl=$cnACL}
       <input type='text' id="cn" name="cn" size=25 maxlength=60 value="{$cn}" title="{t}Name of the NIS Netgroup{/t}">
{/render}
{/if}
     </td>
    </tr>
    <tr>
     <td>
      <LABEL for="description">{t}Description{/t}</LABEL>
     </td>
     <td>
{render acl=$descriptionACL checkbox=$multiple_support checked=$use_description}
      <input type='text' id="description" name="description" size=40 maxlength=80 value="{$description}" title="{t}Descriptive text for this NIS Netgroup{/t}">
{/render}
     </td>
    </tr>
    <tr>
     <td colspan=2> 
      <div style="height:15px;"></div> 
     </td>
    </tr>
    <tr>
     <td>
      <LABEL for="base">{t}Base{/t}</LABEL>{$must}
     </td>
     <td>
{render acl=$baseACL checkbox=$multiple_support checked=$use_base}
       {$base}
{/render}
     </td>
    </tr>
    <tr>
      <td colspan=2> <div style="height:15px; width:100%; border-bottom:1px solid #909090;"></div> </td>
    </tr>
    <tr>
      <td colspan=2> <div style="height:15px; width:100%;"></div> </td>
    </tr>

    <tr>
      <td colspan=2> <div style="height:15px; width:100%; border-bottom:1px solid #909090;"></div> </td>
    </tr>
    <tr>
      <td colspan=2> <div style="height:15px; width:100%;"></div> </td>
    </tr>
   </table>

  </td>
  <td style="border-left:1px solid #A0A0A0">
   &nbsp;
  </td>

  <td style="vertical-align:top;">

   <table summary="" style="width:100%">
    <tr>
     <td style="vertical-align:top; width:50%">
      <b><LABEL for="members">{t}NIS Netgroup members{/t}</LABEL></b>
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

 </tr>
</table>

<input type="hidden" name="nisnetgroupedit" value="1">

<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
  focus_field('cn');
  -->
</script>
