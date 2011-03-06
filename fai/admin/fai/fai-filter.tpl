<div class="contentboxh">
 <p class="contentboxh">
  <img src="images/launch.png" align="right" alt="[F]">{t}Filter{/t}
 </p>
</div>

<div class="contentboxb">

<div style="border-top:1px solid #AAAAAA"></div>

<div style='padding:4px;'>
  <LABEL for='RELEASE'>{t}Release{/t}</LABEL>&nbsp;{$RELEASE}<br>
</div>

<div style="border-top:1px solid #AAAAAA"></div>

<div style='padding:4px;'>

{if $allow_create}
  <input class="center" type="image" name="branch_branch" src="plugins/fai/images/branch_small.png">
  <a href="{$plug}&act=branch_branch">{t}Create release{/t}</a>
  <br>
  <input class="center" type="image" name="freeze_branch" src="plugins/fai/images/freeze.png">
  <a href="{$plug}&act=freeze_branch">{t}Create read-only release{/t}</a>
{else}
  <img src="plugins/fai/images/branch_small_grey.png" alt='{t}Release{/t}' class='center'>
  {t}Create release{/t}
  <br>
  <img src="plugins/fai/images/freeze_grey.png" alt='{t}Freeze{/t}' class='center'>
  {t}Create read-only release{/t}
{/if}

{if $fai_release != $fai_base && $allow_remove}
  <br>
  <input class="center" type="image" name="remove_branch" src="images/lists/trash.png">
  <a href="{$plug}&act=remove_branch">{t}Delete current release{/t}</a>
{/if}

</div>

<div style="border-top:1px solid #AAAAAA"></div>

{$PROFILE}   {t}Show profiles{/t}
<br>{$TEMPLATE}  {t}Show templates{/t}
<br>{$SCRIPT}    {t}Show scripts{/t}
<br>{$HOOK}      {t}Show hooks{/t}
<br>{$VARIABLE}  {t}Show variables{/t}
<br>{$PACKAGE}   {t}Show packages{/t}
<br>{$PARTITION} {t}Show partitions{/t}
{if $opsi_available}
<br>{$NETBOOT} {t}Show netboot products{/t}
<br>{$LOCALBOOT} {t}Show localboot products{/t}
{/if}

 <table summary="" style="width:100%;border-top:1px solid #B0B0B0;">
  <tr>
   <td>
    <label for="NAME">
     <img src="images/lists/search.png" align=middle>&nbsp;{t}Name{/t}
    </label>
   </td>
   <td>
    {$NAME}
   </td>
  </tr>
 </table>

 <table summary=""  width="100%"  style="background:#EEEEEE;border-top:1px solid #B0B0B0;">
  <tr>
   <td width="100%" align="right">
    {$APPLY}
   </td>
  </tr>
 </table>
</div>
