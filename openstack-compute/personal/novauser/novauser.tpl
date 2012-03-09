<table style='width:100%; ' summary="Nova User">

 <tr>
  <td style='width:50%; '>


   <h3>Nova Account</h3>
   <table summary="Account settings">
    <tr>
     <td><label for="isNovaAdmin">Set as Nova Admin</label></td>
     <td>
{render acl=$isNovaAdminACL}
      <input type="checkbox" id="isNovaAdmin" name="isNovaAdmin" value="1" 
       {$isNovaAdminCHK}>
{/render}
     </td>
    </tr>

   </table>

  </td>
  <td class='left-border'>

   &nbsp;
  </td>
  <td style='width:100%; '>


  </td>
 </tr>
</table>

<input type="hidden" name="novauserTab" value="novauserTab">

<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('isNovaAdmin');
  -->
</script>

