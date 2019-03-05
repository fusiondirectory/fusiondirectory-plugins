<div class="contentboxh">
 <p>{t}Filter{/t}</p>
</div>

<div class="contentboxb">
  {if isset($dateFilters)}
    <table>
    {foreach from=$dateFilters item="dfilter"}
      <tr>
        <td><label for="{$dfilter.id}" style="padding:3px;">{$dfilter.label}&nbsp;</label></td>
        <td>{${$dfilter.id}}</td>
      </tr>
    {/foreach}
    </table>
  {/if}

  <hr/>
  {$SCOPE}
  <hr/>

  <label for="NAME" title="{$NAMEDESC}"><img src="geticon.php?context=actions&amp;icon=system-search&amp;size=16" alt="Search"/></label>{$NAME}

  <div>
    {$APPLY}
  </div>
</div>
