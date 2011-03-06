<div class="contentboxh" style="border-bottom:1px solid #B0B0B0;">
 <p class="contentboxh"><img src="{$branchimage}" align="right" alt="[F]">{t}Branches{/t}</p>
</div>
<div class="contentboxb">
 <table summary="" style="width:100%;">
  <tr>
   <td>
    {t}Current release{/t}&nbsp;
    <select name="app_release" onChange="document.mainform.submit();">
        {html_options output=$app_releases values=$app_releases selected=$app_release}
    </select>
   </td>
  </tr>
 </table>
</div>
<br>
