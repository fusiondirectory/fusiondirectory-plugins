<div id="{$sectionId}"  class="plugin-section fullwidth">
  <span class="legend">
    {if $sectionIcon}<img src="{$sectionIcon|escape}" alt=""/>{/if}{$section|escape}
  </span>
  <div>
    {if $attributes.events|@count == 0}
      <p>
      {t}You can import a list of jobs into the FusionDirectory job queue. This should be a semicolon seperated list of items in the following format:{/t}<br/><br/>
      <i>{t}timestamp{/t};{t}MAC-address{/t};{t}job type{/t};[{t}object group{/t}]</i><br/><br/>
      {t 1=$jobtypes}Available job types: %1{/t}<br/>
      </p>
    {else}
      <table style="width: 100%;" class="listingTable">
        <thead>
          <tr>
            <th>{t}Timestamp{/t}</th>
            <th>{t}MAC{/t}</th>
            <th>{t}Event{/t}</th>
            <th>{t}Object group{/t}</th>
            <th>{t}Error{/t}</th>
          </tr>
        </thead>
        <tbody>
          {foreach from=$attributes.events item=event}
            <tr>
              <td>{$event.TIMESTAMP|escape}</td>
              <td>{$event.MAC|escape}</td>
              <td>{$event.HEADER|escape}</td>
              <td>{$event.OGROUP|escape}</td>
              {if $event.ERROR}
                <td style="background-color: #F0BBBB;"><b>{$event.ERROR}</b></td>
              {else}
                <td></td>
              {/if}
            </tr>
          {/foreach}
        </tbody>
      </table>
    {/if}
  </div>
</div>
