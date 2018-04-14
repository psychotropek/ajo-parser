<?php

/**
 * @file
 * Contains FeedsAJOProcessor.
 */

/**
 * Allows a feed node to populate fields on itself.
 */
class FeedsAJOProcessor extends FeedsNodeProcessor {

  /**
   * Overrides parent::process().
   */
  public function process(FeedsSource $source, FeedsParserResult $parser_result) {
    
    //todo: while parser result has more?
    $item = $parser_result->shiftItem();

    //todo: manually create and save node, as in ajo_transform test page
    var_dump($item);
    
    $hash = $this->hash($item);

    try {
      // Load entity.
      $node = $this->entityLoad($source, $nid);
      $this->newItemInfo($node, $node->nid, $hash);

      // Set property and field values.
      $this->map($source, $parser_result, $node);


      
      $this->entityValidate($node);


      // This will throw an exception on failure.
      $this->entitySaveAccess($node);

      $this->entitySave($node);

      //todo: just deal with new nodes
      $this->updateAJOItemInfo($node, $hash);

      // Set message.
      drupal_set_message(t('Updated %title', array('%title' => $node->title)));
      $source->log('import', 'Updated %title', array('%title' => $node->title), WATCHDOG_INFO);
    }
    // Something bad happened, log it.
    catch (Exception $e) {
      drupal_set_message($e->getMessage(), 'warning');
      $message = $this->createLogMessage($e, $node, $item);
      $source->log('import', $message, array(), WATCHDOG_ERROR);
    }
  }


}
