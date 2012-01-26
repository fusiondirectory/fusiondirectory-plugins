<table summary="" width="100%">
 <tr>
  <td style="vertical-align:top; width:50%; border-right:1px solid #B0B0B0">
{if $StandAlone}
   <h2>{t}General{/t}</h2>
   <table summary="">
    <tr>
     <td><LABEL for="cn" >{t}Printer name{/t}</LABEL>{$must}</td>
     <td>
{render acl=$cnACL}
      <input type='text' name="cn" id="cn" size=20 maxlength=60 value="{$cn}">
{/render}
     </td>
    </tr>
    <tr>
     <td><LABEL for="description">{t}Description{/t}</LABEL></td>
     <td>
{render acl=$descriptionACL}
      <input type='text' id="description" name="description" size=25 maxlength=80 value="{$description}">
{/render}
     </td>
    </tr>
    <tr>
      <td colspan=2>&nbsp;</td>
    </tr>
    <tr>
     <td><LABEL for="base">{t}Base{/t}</LABEL>{$must}</td>
     <td>
{render acl=$baseACL}
       {$base}
{/render}
     </td>
    </tr>
   </table>
  </td>
  <td>
{/if}
   <h2>{t}Details{/t}</h2>
   <table summary="">
{if !$StandAlone}
      <tr> 
	 <td><LABEL for="description">{t}Description{/t}</LABEL></td> 
	 <td> 
	{render acl=$descriptionACL} 
	   <input type='text' id="description" name="description" size=25 maxlength=80 value="{$description}"> 
	{/render} 
	 </td> 
      </tr> 
{/if} 
     <tr>
       <td><LABEL for="l">{t}Printer location{/t}</LABEL></td>
       <td>
{render acl=$lACL}
        <input type='text' id="l" name="l" size=30 maxlength=80 value="{$l}">
{/render}
       </td>
     </tr>
     <tr>
       <td><LABEL for="labeledURI">{t}Printer URL{/t}</LABEL>{$must}</td>
       <td>
{render acl=$labeledURIACL}
        <input type='text' id="labeledURI" name="labeledURI" size=30 maxlength=80 value="{$labeledURI}">
{/render}
       </td>
     </tr>
{if $displayServerPath && 0}
    <tr>
     <td>{t}PPD Provider{/t}
     </td>
     <td>
      <input size=30 type='text' value='{$ppdServerPart}' name='ppdServerPart'>
     </td>
    </tr>
{/if}
   </table>
   <table summary="">
    <tr> 
     <td>
      <br>
      {t}Driver{/t}: <i>{$driverInfo}</i>&nbsp;
{render acl=$gotoPrinterPPDACL mode=read_active}
       <input type="submit" name="EditDriver" value="{t}Edit{/t}">
{/render}
{render acl=$gotoPrinterPPDACL}
       <input type="submit" name="RemoveDriver" value="{t}Remove{/t}">
{/render}
     </td>
    </tr>
   </table>
  </td>
 </tr>
</table>

<p class="plugbottom" style="margin-bottom:0px; padding:0px;">&nbsp;</p>

<h2><img class="center" alt="" align="middle" src="images/lists/locked.png"> {t}Permissions{/t}</h2>
<table summary="" width="100%">
 <tr>
  <td style="border-right:1px solid #B0B0B0; width:50%">
   <table style="width:100%" summary=''>
    <tr>
     <td>
      {t}Users which are allowed to use this printer{/t}<br>
{render acl=$gotoUserPrinterACL}
      <select size="1" name="UserMember[]" title="{t}Users{/t}" style="width:100%;height:120px;"  multiple>
       {html_options options=$UserMembers values=$UserMemberKeys}
      </select><br>
{/render}
{render acl=$gotoUserPrinterACL}
      <input type="submit" value="{msgPool type=addButton}"  name="AddUser">
{/render}
{render acl=$gotoUserPrinterACL}
      <input type="submit" value="{msgPool type=delButton}" name="DelUser">
{/render}
     </td>
    </tr>
   </table> 
 
  </td>
  <td>
   <table style="width:100%" summary=''>
    <tr>
     <td>
      {t}Users which are allowed to administrate this printer{/t}<br>
{render acl=$gotoUserPrinterACL}
           <select size="1" name="AdminMember[]" title="{t}Admins{/t}" style="width:100%;height:120px;"  multiple>
                    {html_options options=$AdminMembers values=$AdminMemberKeys}
                   </select><br>
{/render}
{render acl=$gotoUserPrinterACL}
       <input type="submit" value="{msgPool type=addButton}"  name="AddAdminUser">
{/render}
{render acl=$gotoUserPrinterACL}
       <input type="submit" value="{msgPool type=delButton}" name="DelAdmin">
{/render}
  
     </td>
    </tr>
   </table>
   
  </td>
 </tr>
</table>


{if $netconfig ne ''}
<p class="plugbottom" style="margin-bottom:0px; padding:0px;">&nbsp;</p>
{$netconfig}
{/if}

<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">

  <!-- // First input field on page
  if(document.mainform.cn)
	focus_field('cn');
  -->
</script>
