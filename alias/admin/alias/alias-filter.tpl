<div class="contentboxh">
 <p class="contentboxh">
  <img src="images/launch.png" align="right" alt="[F]">{t}Filter{/t}
 </p>
</div>

<div class="contentboxb">

  {$MAILREDIRECTION}<label for="MAILREDIRECTION">&nbsp;{t}Show mail redirections{/t}</label><br>
  {$MAILDISTRIBUTION}<label for="MAILDISTRIBUTION">&nbsp;{t}Show mail distribution{/t}</label><br>
  {if class_available('sympaAlias')}
  {$SYMPAALIAS}<label for="SYMPAALIAS">&nbsp;{t}Show sympa list alias{/t}</label><br>
  {/if}

  <div style="display:block;width=100%;border-top:1px solid #AAAAAA;"></div>

 {$SCOPE}

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
