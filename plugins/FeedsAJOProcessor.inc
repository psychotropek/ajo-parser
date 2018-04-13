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

  /**
   * Updates {feeds_AJO_processor_item}.
   * TODO: we don't have a title for this, even
   * TODO: may be unnecessary
   * 
   * @param object $node
   *   The node being updated.
   * @param string $hash
   *   The hash of the feed item.
   */
  protected function updateAJOItemInfo(stdClass $node, $hash) {
    $insert = !db_query("SELECT 1 FROM {feeds_AJO_processor_item} WHERE feed_nid = :nid", array(':nid' => $node->nid))->fetchField();

    $item = new stdClass();
    $item->feed_nid = $node->nid;
    $item->id = $this->id;
    $item->imported = REQUEST_TIME;
    $item->hash = $hash;
    if ($insert) {
      drupal_write_record('feeds_AJO_processor_item', $item);
    }
    else {
      drupal_write_record('feeds_AJO_processor_item', $item, 'feed_nid');
    }
  }

  /**
   * Overrides parent::getHash().
   * TODO: check table name
   */
  protected function getHash($nid) {
    return (string) db_query("SELECT hash FROM {feeds_AJO_processor_item} WHERE feed_nid = :nid", array(':nid' => $nid))->fetchField();
  }

  /**
   * Overrides parent::getMappingTargets().
   * TODO: add more mapping targets with the other fields
   */
 /**
   * Return available mapping targets.
   */
  public function getMappingTargets() {
    $type = node_type_get_type($this->bundle());

    $targets = parent::getMappingTargets();
    if ($type && $type->has_title) {
      $targets['title'] = array(
        'name' => t('Title'),
        'description' => t('The title of the node.'),
        'optional_unique' => TRUE,
      );
    }
    $targets['nid'] = array(
      'name' => t('Node ID'),
      'description' => t('The nid of the node. NOTE: use this feature with care, node ids are usually assigned by Drupal.'),
      'optional_unique' => TRUE,
    );
    $targets['uid'] = array(
      'name' => t('User ID'),
      'description' => t('The Drupal user ID of the node author.'),
    );
    $targets['user_name'] = array(
      'name' => t('Username'),
      'description' => t('The Drupal username of the node author.'),
    );
    $targets['user_mail'] = array(
      'name' => t('User email'),
      'description' => t('The email address of the node author.'),
    );
    $targets['status'] = array(
      'name' => t('Published status'),
      'description' => t('Whether a node is published or not. 1 stands for published, 0 for not published.'),
    );
    $targets['created'] = array(
      'name' => t('Published date'),
      'description' => t('The UNIX time when a node has been published.'),
    );
    $targets['changed'] = array(
      'name' => t('Updated date'),
      'description' => t('The Unix timestamp when a node has been last updated.'),
    );
    $targets['promote'] = array(
      'name' => t('Promoted to front page'),
      'description' => t('Boolean value, whether or not node is promoted to front page. (1 = promoted, 0 = not promoted)'),
    );
    $targets['sticky'] = array(
      'name' => t('Sticky'),
      'description' => t('Boolean value, whether or not node is sticky at top of lists. (1 = sticky, 0 = not sticky)'),
    );

    $this->getHookTargets($targets);

    return $targets;
  }


}
