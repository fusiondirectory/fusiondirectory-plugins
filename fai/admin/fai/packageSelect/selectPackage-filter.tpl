<div class="contentboxh">
 <p>{t}Filter{/t}</p>
</div>

<div class="contentboxb">
  <div style="padding:3px;">
    {t}A maximum of 200 entries will be shown here.{/t}
  </div>

  {foreach from=$customs key=key item=value}
    {$value}<label for="{$key}">&nbsp;custom {$key}</label><br/>
  {/foreach}

  <hr/>

  <label for="NAME"><img src="geticon.php?context=actions&amp;icon=system-search&amp;size=16"/></label>{$NAME}

  <div>
    {$APPLY}
  </div>
</div>
