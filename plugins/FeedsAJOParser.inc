<?php

/**
 * @file
 * Contains the FeedsAJOParser class.
 */

/**
 * Parses a given file as a CSV file.
 */
class FeedsAJOParser extends FeedsParser {

  /**
   * Implements FeedsParser::parse().
   */

   //TODO: check on what we're fed from the result...how many files?
  public function parse(FeedsSource $source, FeedsFetcherResult $fetcher_result) {
    //todo: handling of non-html files
    $html = $fetcher_result->getRaw();
    $result = new FeedsParserResult();
  //here, parse through things and add them to the result items array
  //$item = array('url' => (string) $url->loc);
  //$result->items[] = $item;


      

    return $result;
  }

  /**
   * Override parent::getMappingSources().
   * n.b. this is false b/c we're hardcoding all of this
   */
  public function getMappingSources() {
    return FALSE;
  }

  /**
   * Define defaults.
   */
  public function sourceDefaults() {
    return false;
  }

  /**
   * Define default configuration.
   * n.b. 
   */
  public function configDefaults() {
    return false;
  }

}
  