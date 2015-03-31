<table width="100%">
<tr>
    <td style="width:100%;vertical-align:top;">
<h2>{t}Zones{/t}</h2>
      {$ZoneList}
      {if $is_createable}
      <input type="submit" name="AddZone" value="{msgPool type=addButton}">
      {else}
      <input type="button" value="{msgPool type=addButton}" disabled>
      {/if}
    </td>
</tr>
</table>

<script type="text/javascript">
  <!-- // First input field on page
  focus_field('AddZone');
  -->
</script>

<p class="seperator">&nbsp;</p>
<p>
<div style="width:100%; text-align:right;">
    <input type='submit' name='SaveService' value='{msgPool type=saveButton}'>
    &nbsp;
    <input type='submit' name='CancelService' value='{msgPool type=cancelButton}'>
</div>
</p>
